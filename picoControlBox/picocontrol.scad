$fn = 50;
wall = 2.5; 
radius = 2;
height = 42;
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
//        translate([width-radius,radius,wall])cylinder(r=radius+wall,h= height);
//        translate([width-radius,depth-radius,wall])cylinder(r=radius+wall,h = height);
//        translate([radius,depth-radius,wall])cylinder(r=radius+wall,h = height); 
}
// and now the holes
//        translate([radius,radius,-0.01])cylinder(r=radius, h= height);
//        translate([width-radius,radius,-0.01])cylinder(r=radius,h= height);
//        translate([width-radius,depth-radius,-0.01])cylinder(r=radius,h = height);
//        translate([radius,depth-radius,-0.01])cylinder(r=radius,h = height); 

        translate([radius,radius,-2])cylinder(d=4, h= height);
        translate([width-radius,radius,-2])cylinder(d=4,h= height);
        translate([width-radius,depth-radius,-2])cylinder(d=4,h = height);
        translate([radius,depth-radius,-2])cylinder(d=4,h = height); 
//
        translate([103,-3,28])cube([18,30,20]);
}
 
