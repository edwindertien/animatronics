$fn = 50;
wall = 3; 
radius = 5;
height = 20;
width = 205;
depth = 125; 

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
        translate([radius,radius,wall])cylinder(r=radius+wall,h = height);
        translate([width-radius,radius,wall])cylinder(r=radius+wall,h= height);
        translate([width-radius,depth-radius,wall])cylinder(r=radius+wall,h = height);
        translate([radius,depth-radius,wall])cylinder(r=radius+wall,h = height); 
}
// and now the holes
        translate([radius,radius,-0.01])cylinder(r=radius, h= height);
        translate([width-radius,radius,-0.01])cylinder(r=radius,h= height);
        translate([width-radius,depth-radius,-0.01])cylinder(r=radius,h = height);
        translate([radius,depth-radius,-0.01])cylinder(r=radius,h = height); 

        translate([radius,radius,height-2])cylinder(d=4, h= height);
        translate([width-radius,radius,height-2])cylinder(d=4,h= height);
        translate([width-radius,depth-radius,height-2])cylinder(d=4,h = height);
        translate([radius,depth-radius,height-2])cylinder(d=4,h = height); 

        translate([75,depth-7,16])rotate([-90,0,0])cylinder(d=5.5,h=20);
        translate([55,depth-7,22])rotate([-90,0,0])cylinder(d=4,h=20); 
        translate([width-1,45,16])cube([20,13.5,20]);
        translate([width/2-2.5,depth-1,22])cube([5,10,2]);
}
 
