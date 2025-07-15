$fn = 50;
wall = 2.5; 
radius = 2;
height = 22;
width = 140;
depth = 56; 

difference(){
union(){
difference(){
    hull(){
        translate([radius,radius,0])cylinder(r=radius+wall,h = height+wall);
        translate([width-radius,radius,0])cylinder(r=radius+wall,h = height+wall);
        translate([width-radius,depth-radius,0])cylinder(r=radius+wall,h = height+wall);
        translate([radius,depth-radius,0])cylinder(r=radius+wall,h = height+wall);
    }
    hull(){
        translate([radius,radius,wall])cylinder(r=radius,h = height+wall);
        translate([width-radius,radius,wall])cylinder(r=radius,h= height+wall);
        translate([width-radius,depth-radius,wall])cylinder(r=radius,h = height+wall);
        translate([radius,depth-radius,wall])cylinder(r=radius,h = height+wall);  
    }  
}
// the standof
//        translate([radius,radius,wall])cylinder(r=radius+wall,h = height);
//      translate([width-radius,radius,wall])cylinder(r=radius+wall,h= height);
//       translate([width-radius,depth-radius,wall])cylinder(r=radius+wall,h = height);
//        translate([radius,depth-radius,wall])cylinder(r=radius+wall,h = height); 
}
// and now the holes
//       translate([radius,radius,-0.01])cylinder(r=radius, h= height);
//        translate([width-radius,radius,-0.01])cylinder(r=radius,h= height);
//        translate([width-radius,depth-radius,-0.01])cylinder(r=radius,h = height);
//        translate([radius,depth-radius,-0.01])cylinder(r=radius,h = height); 

        translate([radius+2,radius+2,-2])cylinder(d=4, h= height);
        translate([width-radius-2,radius+2,-2])cylinder(d=4,h= height);
        translate([width-radius-2,depth-radius-2,-2])cylinder(d=4,h = height);
        translate([radius+2,depth-radius-2,-2])cylinder(d=4,h = height); 
//
        translate([103,-3,6])cube([16,10,14]);

translate([122,-3,6])cube([10,10,11.5]);
translate([28,-3,8])rotate([-90,0,0])cylinder(d=7,h=10);
translate([50,50,14.5])cube([12,10,8]);

translate([35,50,10])rotate([-90,0,0])cylinder(d=6.5,h=10);
}
 
