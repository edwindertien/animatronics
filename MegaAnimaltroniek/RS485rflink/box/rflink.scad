$fn=40;
lenght = 88;
rotate([180,0,0])lid();
//box();
module box(){
difference(){
hull(){
    translate([0,20,0])sphere(d=4);
    translate([0,0,0])sphere(d=4);
    translate([lenght,0,0])sphere(d=4);
    translate([lenght,20,0])sphere(d=4);
        translate([0,20,20])sphere(d=4);
    translate([0,0,20])sphere(d=4);
    translate([lenght,0,20])sphere(d=4);
    translate([lenght,20,20])sphere(d=4);
    
}
cube([lenght,20,20]);
translate([-10,-10,17])cube([lenght+20,40,10]);
translate([-5,1.5,0])cube([10,17,14]);

translate([lenght-1,15,12])rotate([0,90,0])cylinder(d=7,h=20);
//translate([lenght-1,3.75,4])cube([10,12.5,11]);
//translate([25,-3,10])rotate([270,0,0])cylinder(d=8,h=10);
}
}

module lid(){
union(){difference(){hull(){
    translate([0,20,0])sphere(d=4);
    translate([0,0,0])sphere(d=4);
    translate([lenght,0,0])sphere(d=4);
    translate([lenght,20,0])sphere(d=4);
        translate([0,20,20])sphere(d=4);
    translate([0,0,20])sphere(d=4);
    translate([lenght,0,20])sphere(d=4);
    translate([lenght,20,20])sphere(d=4);
    
}
translate([-10,-10,-2])cube([lenght+20,40,18]);
translate([1,1,10])cube([lenght-2,18,10]);
}
difference(){
    translate([0,0,15])cube([lenght,20,2]);
    translate([1,1,14.9])cube([lenght-2,18,2.2]);
}
}
}