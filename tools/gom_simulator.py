#!/usr/bin/env python3
"""GOM-804/GOM-805 serial SCPI simulator.

Use this program at the other end of a virtual serial-port pair (for example
com0com on Windows) while testing the STM32 router.  It deliberately models
the serial *device* only: it does not create virtual COM ports itself.

Example (the STM32/router opens COM10, simulator opens its peer COM11)::

    python tools/gom_simulator.py --port COM11 --model GOM-805 --serial 805001

Install the sole runtime dependency once with ``python -m pip install pyserial``.
"""

from __future__ import annotations

import argparse
import logging
import random
import re
import sys
import time
from collections import deque
from dataclasses import dataclass, field
from typing import Optional


VERSION = "1.0"
OHM_MIN = 5.0e-3
OHM_MAX = 5.0e6


@dataclass
class GomSimulator:
    """Stateful SCPI model, independent of serial transport and easy to test."""

    model: str
    serial_number: str
    firmware: str
    resistance_ohm: float
    noise_ppm: float
    state: dict[str, str] = field(default_factory=dict)
    errors: deque[str] = field(default_factory=deque)

    def __post_init__(self) -> None:
        # Values are stored as SCPI strings so set/query behaviour is repeatable.
        self.state.update({
            "SENS:FUNC": "OHM", "SENS:AUTO": "ON", "SENS:RANG": "AUTO",
            "SENS:SPE": "FAST", "SENS:REL:STAT": "OFF", "SENS:REL:DAT": "0",
            "SENS:REAL:STAT": "OFF", "SENS:DISP": "ON", "TRIG:SOUR": "INT",
            "TRIG:EDGE": "RISING", "TEMP:STAT": "OFF", "TEMP:UNIT": "DEGC",
            "SYST:AVER:STAT": "OFF", "SYST:AVER:DAT": "2", "SYST:LFR": "AUTO",
        })

    def _error(self, code: int, text: str) -> None:
        """Queue an SCPI error; only SYST:ERR? exposes it to the client."""
        self.errors.append(f'{code},"{text}"')

    def _measurement(self) -> str:
        """Return a deterministic-range reading with configurable zero-mean noise."""
        relative_noise = self.noise_ppm * 1.0e-6
        measured = self.resistance_ohm * (1.0 + random.uniform(-relative_noise, relative_noise))
        if self.state["SENS:REL:STAT"] == "ON":
            measured -= float(self.state["SENS:REL:DAT"])
        return f"{measured:.9G}"

    def _is_805_only(self, header: str) -> bool:
        return header.startswith(("BINN", "SOUR:DRY", "SOUR:DRIV", "SYST:PWM"))

    @staticmethod
    def _normalise(command: str) -> tuple[str, Optional[str], bool]:
        """Split one SCPI command into upper-case header, optional argument and query flag."""
        command = command.strip().upper()
        if not command:
            return "", None, False
        pieces = command.split(None, 1)
        header = pieces[0]
        argument = pieces[1].strip() if len(pieces) == 2 else None
        query = header.endswith("?")
        return (header[:-1] if query else header), argument, query

    def handle(self, command: str) -> Optional[str]:
        """Process one CR/LF-delimited command and return a response for queries only."""
        header, argument, query = self._normalise(command)
        if not header:
            return None
        if ";" in command:
            self._error(-108, "One command per message required")
            return None
        if header in ("*IDN", "SYST:DEV:IDN") and query:
            return f"GOM,{self.model},{self.serial_number},{self.firmware}"
        if header == "*TST" and query:
            return "0"
        if header in ("SYST:ERR", "SYST:DEV:ERR") and query:
            return self.errors.popleft() if self.errors else '0,"No error"'
        if header == "*RST" and not query and argument is None:
            self.state.clear(); self.__post_init__()
            return None
        if header in ("READ", "MEAS:RES") and query:
            return self._measurement()
        if header == "*TRG" and not query:
            return None
        if self._is_805_only(header) and self.model != "GOM-805":
            self._error(-113, "Undefined header")
            return None

        # All command families whitelisted by the STM32 project.  BINN# is a
        # numbered bin (1..8); the remaining entries accept both set and query.
        known = re.match(
            r"^(SENS:(FUNC|AUTO|RANG|SPE|REL:STAT|REL:DAT|REAL:STAT|DISP)|"
            r"TRIG:(SOUR|DEL:STAT|DEL:DAT|EDGE)|"
            r"CALC:COMP:(TYPE|LIM:(REF|MODE|LOW|UPP|RES)|PERC:(LOW|UPP)|BEEP|MATH:DAT)|"
            r"BINN([1-8])?:(COUN:(CLE|TOT|OUT|RES)|LIM:(LOW|UPP|BEEP|DISP|MODE|REF|RES)|PERC:(LOW|UPP))|"
            r"TEMP:(COMP:(CORR|COEF)|CONV:(RES|TEMP|CONS|DISP|MATH:DAT)|STAT|DAT|UNIT|AMB:(STAT|DAT))|"
            r"SYST:(AVER:(STAT|DAT)|MDEL:(STAT|DAT)|LFR|PWM:(ON|OFF))|SOUR:(DRY|DRIV))$",
            header,
        )
        if not known:
            self._error(-113, "Undefined header")
            return None
        if query:
            # Result-style queries return a measurement/status; configuration
            # queries return their last configured value or a safe default.
            if header.endswith(("MATH:DAT", "LIM:RES", "COUN:RES", "TEMP:DAT")):
                return self._measurement()
            return self.state.get(header, "0")
        if argument is None:
            # Clear counters and triggers are valid write commands with no data.
            if header.endswith("COUN:CLE"):
                self.state[header] = "0"
                return None
            self._error(-109, "Missing parameter")
            return None
        if header == "SENS:RANG":
            try:
                value = float(argument)
            except ValueError:
                self._error(-128, "Numeric data error")
                return None
            if not OHM_MIN <= value <= OHM_MAX:
                self._error(-222, "Data out of range")
                return None
        self.state[header] = argument
        return None


def run_serial(simulator: GomSimulator, port: str, baudrate: int) -> None:
    """Serve one serial endpoint forever; each response is CR/LF terminated."""
    try:
        import serial  # Imported here so --help works before pyserial is installed.
    except ImportError as exc:
        raise SystemExit("Missing dependency: python -m pip install pyserial") from exc

    with serial.Serial(port, baudrate=baudrate, bytesize=8, parity="N", stopbits=1,
                       timeout=0.1, write_timeout=1) as link:
        logging.info("%s %s listening on %s at %d 8N1", simulator.model,
                     simulator.serial_number, port, baudrate)
        buffer = bytearray()
        while True:
            data = link.read(128)
            if not data:
                continue
            for byte in data:
                if byte == 10:  # LF terminates one SCPI program message.
                    command = buffer.rstrip(b"\r").decode("ascii", errors="replace")
                    buffer.clear()
                    logging.info("RX %s", command)
                    reply = simulator.handle(command)
                    if reply is not None:
                        link.write((reply + "\r\n").encode("ascii"))
                        link.flush()
                        logging.info("TX %s", reply)
                elif len(buffer) < 384:
                    buffer.append(byte)
                else:
                    # Recover only at LF, matching a robust embedded line parser.
                    simulator._error(-363, "Input buffer overrun")


def main() -> None:
    parser = argparse.ArgumentParser(description="Serial SCPI simulator for GOM-804/GOM-805")
    parser.add_argument("--port", required=True, help="Simulator-side COM port, e.g. COM11")
    parser.add_argument("--baud", type=int, default=115200, help="Baud rate (default: 115200)")
    parser.add_argument("--model", choices=("GOM-804", "GOM-805"), default="GOM-805")
    parser.add_argument("--serial", default="SIM0001", help="Value returned by *IDN?")
    parser.add_argument("--firmware", default="SIM-1.0", help="Firmware value returned by *IDN?")
    parser.add_argument("--resistance", type=float, default=1.0, help="Nominal resistance in ohms")
    parser.add_argument("--noise-ppm", type=float, default=10.0, help="Peak measurement noise in ppm")
    parser.add_argument("--verbose", action="store_true")
    args = parser.parse_args()
    if not OHM_MIN <= args.resistance <= OHM_MAX:
        parser.error(f"--resistance must be within {OHM_MIN}..{OHM_MAX} ohm")
    if args.noise_ppm < 0:
        parser.error("--noise-ppm must be non-negative")
    logging.basicConfig(level=logging.DEBUG if args.verbose else logging.INFO, format="%(asctime)s %(message)s")
    run_serial(GomSimulator(args.model, args.serial, args.firmware, args.resistance, args.noise_ppm),
               args.port, args.baud)


if __name__ == "__main__":
    main()
