

$fn = 100;
wall = 3;

module handle(){
    difference(){union(){
difference(){
cylinder(d=110,h=8);
    cylinder(d=78,h=9);
    rotate([0,0,-50])cube([80,80,80]);
    rotate([0,0,140])cube([80,80,80]);
    translate([-80,0,0])cube([160,80,80]);
}
translate([47*sin(140),47*cos(140),0])cylinder(d=16,h=8);
translate([47*sin(-140),47*cos(-140),0])cylinder(d=16,h=8);
}
translate([-30,-27,-32])rotate([15,0,0])bolt();
translate([30,-27,-32])rotate([15,0,0])bolt();

}}

//translate([0,0,0])lid();
difference(){
union(){
base();
translate([27,34.5,3])rotate([0,3.6,0])rotate([90,0,90])transmitter();
    
    
    
    }
translate([27,34.5,3])rotate([0,3.6,0])rotate([90,0,90])transmittercutout();
}


module transmitter(){
difference(){
    cube([45+6,60+6,20+3]);
transmittercutout();
}}

module transmittercutout(){
        translate([3,3,3])cube([45,60,40]);
    translate([1.5,25,10.5])cube([3,16,10]);
    translate([45+1.5,25,10.5])cube([3,16,10]);
    translate([41,5,-0.01])cube([4,15,10]);
}

module bolt(){
    cylinder(d=5,h=40);
    translate([0,0,40])cylinder(d=8,h=5);
}


//translate([-30,-24,0])rotate([15,0,0])color("silver")bracket(5);
//translate([-30,120+24,0])rotate([-15,0,0])color("silver")bracket(5);

//translate([0,0,-110])base();


module base(){
    difference(){union(){
hull(){
cylinder(d1=100, d2 = 110, h=80);
translate([0,120,0])cylinder(d1=100, d2 = 110, h=80);
}
translate([-51.2,-20,20])rotate([0,-3.6,0])cube([10,40,60]);
translate([-51.2,100,20])rotate([0,-3.6,0])cube([10,40,60]);

}
translate([-40.2,0,37])rotate([0,-93.6,0])cylinder(d=5.5,h=30);
translate([-40.2,120,37])rotate([0,-93.6,0])cylinder(d=5.5,h=30);
translate([0,0,wall])hull(){
cylinder(d1=100-2*wall, d2 = 110-2*wall, h=80);
translate([0,120,0])cylinder(d1=100-2*wall, d2 = 110-2*wall, h=80);
}

translate([0,0,75])hull(){
    cylinder(d=110-wall, h=10);
    translate([0,120,0])cylinder(d=110-wall, h=10);
}


}
}

module lid(){
color("red"){
    
 difference(){   
    hull(){
cylinder(d1=110, d2 = 100, h=20);
translate([0,120,0])cylinder(d1=110, d2 = 100, h=20);
}
stick();
translate([0,120,0])stick();

translate([16,60-16,0])cylinder(d=7,h=40);
translate([-16,60-16,0])cylinder(d=7,h=40);
translate([16,60+16,0])cylinder(d=7,h=40);
translate([-16,60+16,0])cylinder(d=7,h=40);

translate([-36,0,0])cylinder(d=12,h=40);
translate([-36,24,0])cylinder(d=12,h=40);

translate([-36,120,0])cylinder(d=12,h=40);
translate([-36,120-24,0])cylinder(d=12,h=40);

    translate([0,0,-0.01])hull(){
cylinder(d1=110-wall, d2 = 100-wall, h=20-wall);
translate([0,120,0])cylinder(d1=110-wall, d2 = 100-wall, h=20-wall);
        }
translate([-30,-24,0])rotate([15,0,0])color("silver")bracket(5);
translate([-30,120+24,0])rotate([-15,0,0])color("silver")bracket(5);

}

translate([0,0,-5])difference(){hull(){
    cylinder(d=110-wall, h=10);
    translate([0,120,0])cylinder(d=110-wall, h=10);
}
translate([0,0,-0.01])hull(){
    cylinder(d=110-wall-wall, h=20);
    translate([0,120,0])cylinder(d=110-wall-wall, h=20);
}
}

difference(){
    translate([-30,-24,0])rotate([15,0,0])bracket(12);
    translate([-30,-24,0])rotate([15,0,0])bracket(5);
    translate([-50,-50,20.00])cube([100,100,50]);
}
difference(){
    translate([-30,120+24,0])rotate([-15,0,0])bracket(12);
    translate([-30,120+24,0])rotate([-15,0,0])bracket(5);
    translate([-50,120-50,20.00])cube([100,100,50]);
}

}//color
}//module





module stick()
{
    cylinder(d=41,h=40);
    translate([-32.5/2,32.5/2,0])cylinder(d=2.7,h=40);
    translate([-32.5/2,-32.5/2,0])cylinder(d=2.7,h=40);
    translate([32.5/2,-32.5/2,0])cylinder(d=2.7,h=40);
    translate([32.5/2,32.5/2,0])cylinder(d=2.7,h=40);
}

module bracket(th){
    translate([0,0,0])cylinder(d=th,h=40);
    translate([60,0,0])cylinder(d=th,h=40);
    translate([0,0,40])sphere(d=th);
    translate([60,0,40])sphere(d=th);
    translate([0,0,40])rotate([0,90,0])cylinder(d=th,h=60);
}