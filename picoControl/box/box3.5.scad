$fn=40;



difference(){
    union(){
        box(54,61,24,2);
            translate([5,4.5,-0.01]){
    translate([0,0,0])cylinder(d=6,h=4.4);
    translate([44,0,0])cylinder(d=6,h=4.4);
    translate([44,52,0])cylinder(d=6,h=4.4);
    translate([0,52,0])cylinder(d=6,h=4.4);
    }}
        
    // power socket    
    translate([50,45,6.6])cube([7,9.5,11]);
    // flatcable
    translate([50,5,11])cube([7,18,8]);
    // screw terminal
    translate([24,60,6.6])cube([16,20,5]);
    // antenne
    translate([7,52,11])rotate([-90,0,0])cylinder(d=7,h=30);
    // minijack
    translate([1,13.7,9])rotate([0,-90,0])cylinder(d=6.4,h=10);
    
    hull(){
        translate([1,8.5,19.5])rotate([0,-90,0])cylinder(d=6,h=10);
        translate([1,15.5,19.5])rotate([0,-90,0])cylinder(d=6,h=10);
    }
    
    translate([-5,28.5,16])cube([30,11,9]);
    
    translate([5,4.5,-0.01]){
    translate([0,0,0])cylinder(d=3.5,h=10);
    translate([44,0,0])cylinder(d=3.5,h=10);
    translate([44,52,0])cylinder(d=3.5,h=10);
    translate([0,52,0])cylinder(d=3.5,h=10);
    }
}

module box(width, length, height, wall){
    difference(){
        hull(){
            translate([0,0,0])cylinder(r=wall,h=height);
            translate([width,0,0])cylinder(r=wall,h=height);
            translate([width,length,0])cylinder(r=wall,h=height);
            translate([0,length,0])cylinder(r=wall,h=height);
        }
        translate([0,0,wall])cube([width,length,height]);
    }
}
        
       