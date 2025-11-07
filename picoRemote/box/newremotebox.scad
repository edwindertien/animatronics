$fn = 40;

//keypad();


//translate([0,0,40])top();
//box();
projection(cut = false)top();

module top()color("black", alpha = 0.3){
    
    difference(){
        hull(){
                translate([0,0,0])cylinder(d=18,h=2);
        translate([180,0,0])cylinder(d=18,h=2);
        translate([180,90,0])cylinder(d=18,h=2);
        translate([0,90,0])cylinder(d=18,h=2);
    }
    
           translate([-2,-2,-0.01])cylinder(d=3,h=36);
        translate([182,-2,-0.01])cylinder(d=3,h=36);
        translate([182,92,-0.01])cylinder(d=3,h=36);
        translate([-2,92,-0.01])cylinder(d=3,h=36); 
    translate([10,24,-0.01])keypad();
}
}


module keypad(){
    //46 x 57, R4
    
    //46 , 59 (r1)
    hull(){
        translate([0,0,0])cylinder(d=8,h=10);
        translate([46-8,0,0])cylinder(d=8,h=10);
         translate([46-8,57-8,0])cylinder(d=8,h=10);
        translate([0,57-8,0])cylinder(d=8,h=10);
}
        translate([-4,-5,0])cylinder(d=2.4,h=10);
        translate([-4,54,0])cylinder(d=2.4,h=10);
        translate([42,-5,0])cylinder(d=2.4,h=10);
         translate([42,54,0])cylinder(d=2.4,h=10);
        

}

module box(){
difference(){
    hull(){
        translate([0,0,0])cylinder(d=20,h=36);
        translate([180,0,0])cylinder(d=20,h=36);
        translate([180,90,0])cylinder(d=20,h=36);
        translate([0,90,0])cylinder(d=20,h=36);
    }
    hull(){
                translate([0,0,2.5])cylinder(d=14,h=36);
        translate([180,0,2.5])cylinder(d=14,h=36);
        translate([180,90,2.5])cylinder(d=14,h=36);
        translate([0,90,2.5])cylinder(d=14,h=36);
    }
    
        hull(){
                translate([0,0,34])cylinder(d=18,h=36);
        translate([180,0,34])cylinder(d=18,h=36);
        translate([180,90,34])cylinder(d=18,h=36);
        translate([0,90,34])cylinder(d=18,h=36);
    }
    translate([72,33,-0.01])cube([45,60,40]);
    
    translate([10,-11,32])rotate([-90,0,0]){
            translate([12,14.5,0])cylinder(d=24,h=5);
    translate([2.5,26.5,0])cylinder(d=3.2,h=5);
    translate([2.5+19,26.5-24,0])cylinder(d=3.2,h=5);
}
    
}
        
translate([120,30,23])rotate([0,180,0])transmitter();

difference(){
union(){
        translate([-2,-2,0])cylinder(d=10,h=34);
        translate([-7,-2,0])cube([5,5,34]);
        translate([-2,-7,0])cube([5,5,34]);
    
        translate([182,-2,0])cylinder(d=10,h=34);
        translate([182,-2,0])cube([5,5,34]);
        translate([182-5,-7,0])cube([5,5,34]);
    
    
        translate([182,92,0])cylinder(d=10,h=34);
        translate([182,92-5,0])cube([5,5,34]);
        translate([182-5,92,0])cube([5,5,34]);
    
        translate([-2,92,0])cylinder(d=10,h=34);
            translate([-7,92-5,0])cube([5,5,34]);
        translate([-2,92,0])cube([5,5,34]);
}    
    
        translate([-2,-2,20])cylinder(d=3,h=36);
        translate([182,-2,20])cylinder(d=3,h=36);
        translate([182,92,20])cylinder(d=3,h=36);
        translate([-2,92,20])cylinder(d=3,h=36);
}
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