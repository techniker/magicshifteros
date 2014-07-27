/*
 * MagicShifter Case Prototype
 * Copyright (C) 2013  Philipp Tiefenbacher <wizards23@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; eitd2her version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,lp
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

// the name of the file that is used for all 2D geometry
dxfFileName = "magicshifter_case_95_bike.dxf";

// if true build plate will be generated; if false assembled model is generated 	
forPrint = true;

preview = false;

// set to true or false depending on what part you want to make
generateBottomPart	= true;
generateTopPart 	= true;
generateButtonCover = false;

// print quality/detail level of cirlces 
$fn=preview ? 8 : 32;

// height of the bottom layer of the bottom part (which is the side you see when you look at the LEDs
floorH=0.5;

// height of the 2nd layer from the bottom (the layer the pcb rests on); it has holes where components are on the bottom side of the PCB

foilH = 0.3;
pcbToFloorH = 2.5 + 0.75 + foilH - 0.6;

// height of the magicshifter pcb (without components)
pcbHeight=1.75;

// the height of the lid (lid == topPart)
lidH = 1.6;

lidHollow = 1.1;

// total height of assembled case
totalH=14; 

pillarCutH = 5.5;


// should be larger than the largest dimension of the case (used in the 2D to 3D conversion), no need to change this 
infSize = 500;

///////////////////////////////////////////////////////////////////////

// height of the button cover (derived from totalH and the other heights)
buttonH = totalH - (pcbToFloorH+floorH+pcbHeight+lidH);

pillarH = totalH-(pcbToFloorH+floorH+pcbHeight+0.3);

////////////////////////////////////////
// the actual parts
////////////////////////////////////////

gap = 3.5;

rotate([0,-90,0])
union()
		{
		
			difference() {
				

			// the pcb rests on this 
				extrudeLayer("bike_outher", totalH-2 + gap, -3 - gap);
				extrudeLayer("bike_inner", totalH+gap, -gap);
				// cut out indents in the case where accessories are inteded to snap fit onto
				#translate([0,0,totalH/2]) fromDXF_XY_XZ("bike_outher", "bike_button");
				
			}
			// main part of case
			difference() 
			{
				// cut out indents in the case where accessories are inteded to snap fit onto
				#translate([0,0,totalH/2]) fromDXF_XY_XZ("bike_snap_xy", "bike_snap_xz");		
			}
		}

/////////////////////////////////////////
// main modules
/////////////////////////////////////////

module bottomPart()
{
	difference()
	{
		union()
		{
			// the pcb rests on this 
			extrudeLayer("bottom_floor_pcb", floorH+pcbToFloorH,0);
	
			// main part of case
			difference() 
			{
				// outher shape of case
				union() {
					extrudeLayer("bottom_floor", totalH, 0);
					translate([0,0,pcbToFloorH+floorH+pcbHeight]) fromDXF_XY_XZ("button_text_xy", "button_text_xz");
				}
				// cut out the size of pcb
				extrudeLayer("pcb_cutout", totalH, floorH);
				//#extrudeLayer("button_cutout", totalH, pcbToFloorH+floorH+pcbHeight+0.6);

#translate([0,0,pcbToFloorH+floorH+pcbHeight]) fromDXF_XY_XZ("button_cutout_xy", "button_cutout_xz");
	
				// cut out buttons and USB connector extra space
				//*extrudeLayer("side_cutouts", totalH, floorH+pcbToFloorH+pcbHeight);
				
				// cut out indents in the case where accessories are inteded to snap fit onto
				translate([0,0,totalH/2]) fromDXF_XY_XZ("schnappen_halter_xy", "schnappen_halter_xz");		
			}
		}
		// snap cutouts
		union() 
		{
			translate([0,0,pcbToFloorH+floorH+pcbHeight]) fromDXF_XY_XZ("button_slits_xy", "button_slits_xz");
			// we need this intersection because to limit the cutout to the height of the lid
			*intersection()
			{
				// the cutout on the sides where the lid should snap fit in
				translate([0,0,totalH]) fromDXF_XY_XZ("lid_snap_cut_xy", "lid_snap_cut_xz");
				// limit the cutout to height of lid 
				cubeEx([infSize,infSize,lidH*2], [0,0,0.5], [0,0,totalH-lidH]); 
			}
			// hole in case for USB connector also includes small holes for attaching a string
			// TODO: hackish because of the positioning, clean up
			extrudeLayerLeft("bottom_cut_left_zy", 10, zOffset=floorH+pcbToFloorH+pcbHeight, xOffset=0);
			// cut out the space where the lid fits in on top
			extrudeLayer("lid_cut", totalH, totalH-lidH);
			*translate([0,0,totalH-lidH]) fromDXF_XY_ZY("lidholder_cutout_xy", "lidholder_cutout_zy");
			extrudeLayer("button_freedom", totalH, totalH-(lidH+0.3));
		}

		//#extrudeLayer("button_indent_xy", lidH,pcbToFloorH+floorH);
//#extrudeLayer("button_indent_xz", 3*totalH,-1);
		translate([0,0,pcbToFloorH+floorH+pcbHeight])  fromDXF_XY_XZ("button_indent_xy", "button_indent_xz", $fn=64);


	// screwholes 
		for (pos = cylPositions) {
			translate([pos[0], pos[1], floorH]) {
				cylinder(r= 1.05, h = totalH, center=false);
			}
		}
	}
}

cylPositions = [
//[85.8,9.5],
  [86.803053, 10.45868], // at bowwerbutton
 [92.8355, -10.1573],
 [18.4147, 10.4587],
 //[8.405193, -10.15731],
 [28.405193, -10.15731],
 //[6.34989, -8.25234]. // over reset
];

microButtonPositions = [
[1.418143, -9.839815],
[97.3138, 3.654232]
];



module topPart()
{
	difference() {
		union()
		{
			// this intersection is needed to get a clean model
			difference()
			{
				intersection()
				{
					union()
					{
						// the main part of the lid
						extrudeLayer("lid", 4*lidH,-2*lidH);
						// the small snapping parts on the side
						*fromDXF_XY_XZ("lid_snap_xy", "lid_snap_xz");
						*translate([0,0,lidH]) mirror([0, 0, 1]) fromDXF_XY_ZY("lidholder_xy", "lidholder_zy");
					}
					// limit bottom pat of the lid to lidH
					cubeEx([infSize,infSize,lidH], [0,0,0.5], [0,0,0]); 
				}
				extrudeLayer("lid_hollow", 10,lidH-lidHollow);
			}
			difference()
			{
				extrudeLayer("pillars", pillarCutH,0);
				//extrudeLayer("pillars", totalH-(pcbToFloorH+floorH+pcbHeight),0);
				//extrudeLayer("pillars_cutaway", totalH, pillarCutH);
			}
			intersection()
			{
				
				for (pos = cylPositions) {
					translate([pos[0], pos[1], lidH/2]) cylinder(r1= 4.5+0.3, r2=0, h = 6+0.3);
					translate([pos[0], pos[1], lidH/2]) cylinder(r= 2.6, h = pillarH);
				}

				extrudeLayer("lid_crop", pillarH, 0);
			}
			
		}
		// screwholes and versenkte schraubenkoepfe ;)
		for (pos = cylPositions) {
			translate([pos[0], pos[1], -1]) {
				cylinder(r= 1.25, h = 3*totalH, center=true);
				cylinder(r1= 3.4+0.3, r2= 0, h = 3.4+0.3, center=false);
			}
		}
		for (pos = microButtonPositions) {
			translate([pos[0], pos[1], -1]) {
				cylinder(r= 1.25, h = 3*totalH, center=true);
			}
		}

if (!preview)
{
		extrudeLayer("logo", 1.3,-1, $fn=16);

		extrudeLayer("logo2", 1.3,-1, $fn=16);
}

			extrudeLayer("button_indent_xy", 3*totalH,-1, $fn=64);
	}
}

/////////////////////////////////////////
// helper modules
/////////////////////////////////////////

/* 
constructs a 3D object by extruding the given layers of the dxf file 
layerXY is extruded in Z axis
layerZY is extruded in the X axis
*/
module fromDXF_XY_ZY(layerXY, layerZY)
{
	intersection()
	{
		linear_extrude(convexity = 10, height = infSize, center = true)
			import_dxf(file = dxfFileName, layer = layerXY, convexity = 10, scale=1);
		rotate([0,-90,0]) linear_extrude(convexity = 10, height = infSize, center = true)
			import_dxf(file = dxfFileName, layer = layerZY, convexity = 10, scale=1);
	}
}

module fromDXF_XY_XZ(layerXY, layerXZ)
{
	intersection()
	{
		linear_extrude(convexity = 10, height = infSize, center = true)
			import_dxf(file = dxfFileName, layer = layerXY, convexity = 10, scale=1);
		rotate([90,0,0]) linear_extrude(convexity = 10, height = infSize, center = true)
			import_dxf(file = dxfFileName, layer = layerXZ, convexity = 10, scale=1);
	}
}
module fromDXF_XY_ZY_XZ(layerXY, layerZY, layerXZ)
{
	intersection()
	{

linear_extrude(convexity = 10, height = infSize, center = true)
			import_dxf(file = dxfFileName, layer = layerXY, convexity = 10, scale=1);

rotate([0,-90,0]) linear_extrude(convexity = 10, height = infSize, center = true)
			import_dxf(file = dxfFileName, layer = layerZY, convexity = 10, scale=1);

rotate([90,0,0]) linear_extrude(convexity = 10, height = infSize, center = true)
			import_dxf(file = dxfFileName, layer = layerXZ, convexity = 10, scale=1);
	}
}

// extrude 2d layer in z axis
module extrudeLayer(layerName, height, zOffset = 0)
{
	translate([0,0,zOffset + (height< 0 ? height : 0)])
		linear_extrude(convexity = 5, height = abs(height), center = false)
			import(file = dxfFileName, layer = layerName, convexity = 6, scale=1);
}

// extrude 2d layer in x axis in left direction;
module extrudeLayerLeft(layerName, height, zOffset = 0, xOffset=0)
{
	translate([xOffset,0,zOffset])
		rotate([0,90,0]) 
				translate([0,0,-height/2]) extrudeLayer(layerName, height);
}

/*
my improved version of the openSCAD cube module
* size works like the first parameter of the cube primitive, it's a 3 dim vector that contains the sidelengths of the "cube"
* center is also a 3 dim vector so you can specify the centering for each axis individually, This is normaly either -0.5, 0 (equal to center = true), or 0.5 (equal to center = false) but any value is possible.
offset specifies an absolute offset in mm from the position that would result from just the center parameter
*/
module cubeEx(size=[1,1,1], center=[0,0,0], offset=[0,0,0])
{
	translate([center[0]*size[0]+offset[0], center[1]*size[1]+offset[1], center[2]*size[2]+offset[2]])
		cube(size, center = true);
}


