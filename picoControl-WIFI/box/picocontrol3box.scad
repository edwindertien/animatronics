$fn=50;


//box(61,54,20,2.5);

translate([0,0,30])lid(61,54,3,2.5);
module box(width, depth, height, wall){
    difference(){
        hull(){
            translate([0,0,0])cylinder(r=wall,h=wall+height);
            translate([width,0,0])cylinder(r=wall,h=wall+height);
            translate([width,depth,0])cylinder(r=wall,h=wall+height);
            translate([0,depth,0])cylinder(r=wall,h=wall+height);
        }
        translate([0,0,wall])cube([width,depth,height+0.01]);
        
        translate([width-7-9,-wall-1,wall+4])cube([10,10,11]);
        translate([width-10,depth,wall+10])rotate([-90,0,0])cylinder(d=7,h=20);
        translate([28,depth,14+wall])cube([12,20,10]);
    }
    
}

module lid(width,depth,height,wall){
    difference(){
        union(){hull(){
            translate([0,0,0])cylinder(r=wall,h=height);
            translate([width,0,0])cylinder(r=wall,h=height);
            translate([width,depth,0])cylinder(r=wall,h=height);
            translate([0,depth,0])cylinder(r=wall,h=height);
            translate([0,0,height])sphere(r=wall);
            translate([width,0,height])sphere(r=wall);
            translate([width,depth,height])sphere(r=wall);
            translate([0,depth,height])sphere(r=wall);
        }
        translate([0,0,-1])cube([width,depth,height]);}
        translate([1,1,-1.01])cube([width-2,depth-2,height+1.01]);
        
        translate([17.5,15.5,0])cube([26,11,30]);
        translate([4,50,-3])cube([5,20,4]);
    }
}
        