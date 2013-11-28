/*
 * dock.h
 *
 * Created: 4/30/2013 3:16:17 PM
 *  Author: mlohse
 */ 


#ifndef DOCK_H_
#define DOCK_H_

#include <stdint.h>

typedef enum
{
	ManualDocking = 0,
	AutoDocking = 1
} DockingType;

typedef enum
{
	DockedManually = 0x00,
	DockedBottom = 0x01,
	DockedTop = 0x02,
	DockedLimit = 0x03,
	DockMovingUp = 0x04,
	DockMovingDown = 0x05,
	DockError = 0xFF
} DockingState;


void dock_init();
void dock_tare();
void dock_update_force();
void dock_parse_forcedata();
void dock_check_limits();
void dock_set_docktype(DockingType type);
void dock_set_dockinglimits(int16_t lower, int16_t upper);

#endif /* DOCK_H_ */
