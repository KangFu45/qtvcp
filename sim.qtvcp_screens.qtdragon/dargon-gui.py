#!/usr/bin/env python

from vismach import *
import hal

c = hal.component("dargon-gui")
c.newpin("joint-x", hal.HAL_FLOAT, hal.HAL_IN)
c.newpin("joint-y", hal.HAL_FLOAT, hal.HAL_IN)
c.newpin("joint-z", hal.HAL_FLOAT, hal.HAL_IN)
c.ready()

###################

tooltip = Capture()

tool = Collection([
	tooltip,
	#Color([0.5,0.5,0.5,0],
	CylinderZ(0,0.3,10,5),
	CylinderZ(10,5,30,5)	
])
#tool = Translate([tool],0,0,-15)

yyy = Collection([
	tool,
	Color([0,1,0,0],[Box(5,10,15,-5,-190,25)])
])

yyy = Translate([yyy],0,0,150)
yyy = HalTranslate([yyy],c,"joint-y",0,1,0)
yyy = HalTranslate([yyy],c,"joint-z",0,0,-1)

yyy = Collection([
	yyy,
	Color([0,0,1,0],[Box(5,-5,190,15,-15,-10)])
])
yyy = HalTranslate([yyy],c,"joint-x",1,0,0)

yyy = Collection([
	yyy,
	Color([1,0,0,0],[Box(-15,-5,-10,485,-15,-20)])
])

work = Capture()

xxx = Collection([
	work,
	Color([0.5,0.5,0.5,0],[CylinderX(0, 30, 450, 30)])
])

xxx = Translate([xxx],0,100,100)

model = Collection([
	yyy,
	xxx
])

main(model, tooltip, work, 1500)
