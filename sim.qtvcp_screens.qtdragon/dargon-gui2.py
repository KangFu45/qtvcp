#!/usr/bin/env python

from vismach import *
import hal

c = hal.component("dargon-gui2")
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

zzz = Collection([
	tool,
	Color([0,1,0,0],[Box(15,-5,20,-15,-25,520)])
])

zzz = Translate([zzz],0,0,400)
zzz = HalTranslate([zzz],c,"joint-z",0,0,-1)
zzz = HalTranslate([zzz],c,"joint-x",1,0,0)

xxx = Collection([
	zzz,
	Color([0,0,1,0],[Box(555,-25,420,-45,-40,440)])
])
xxx = HalTranslate([xxx],c,"joint-y",0,1,0)

xxx = Collection([
	xxx,
	Color([1,0,0,0],[Box(-40,-45,420,-20,555,400)])
])

work = Capture()

yyy = Collection([
	work,
	Color([0.5,0.5,0.5,0],[Box(0,0,0,500,500,10)])
])

model = Collection([
	xxx,
	yyy
])

main(model, tooltip, work, 1500)
