$fn = 50;
wall = 2.5; 
radius = 2;
height = 22;
width = 140;
depth = 56; 
// for the lid: 
//projection(cut=false)box();

box();

module box()difference(){
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
        translate([104,-3,6])cube([16,10,14]);

translate([122,-3,6])cube([10,10,11.5]);
translate([32,-3,9])rotate([-90,0,0])cylinder(d=7,h=10);
translate([50,50,15])cube([12,10,8]);

translate([32,50,10])rotate([-90,0,0])cylinder(d=6.5,h=10);
translate([-1,2,16])cube([2,50,30]);
}
 
