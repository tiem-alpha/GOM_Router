#include "router_application.h"

#include "main.h"
#include "gom_firmware.h"
#include "protocol_limits.h"
#include "router_scpi.h"

#include "cmsis_os2.h"
#include "FreeRTOS.h"
#include "task.h"

extern WWDG_HandleTypeDef hwwdg;

#define ROUTER_TASK_STACK_WORDS 768u

static StaticTask_t router_task_control_block;
static uint32_t router_task_stack[ROUTER_TASK_STACK_WORDS];

void router_application_init(void)
{
    router_scpi_init();
    gom_firmware_init(router_scpi_gom_complete);
}

osThreadId_t router_application_create_task(osThreadFunc_t entry,
                                            const osThreadAttr_t *attributes)
{
    osThreadAttr_t static_attributes = *attributes;

    static_attributes.cb_mem = &router_task_control_block;
    static_attributes.cb_size = sizeof(router_task_control_block);
    static_attributes.stack_mem = router_task_stack;
    static_attributes.stack_size = sizeof(router_task_stack);
    return osThreadNew(entry, NULL, &static_attributes);
}

void router_application_task(void *argument)
{
    char line[GOM_PC_LINE_MAX + 1u];

    (void)argument;
    for (;;) {
    gom_firmware_step();
        if (gom_firmware_ready() && gom_firmware_take_pc_line(line, sizeof(line))) {
            router_scpi_input(line);
        }
        (void)HAL_WWDG_Refresh(&hwwdg);
        osDelay(1u);
    }
}
