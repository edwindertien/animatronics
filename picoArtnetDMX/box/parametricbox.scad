$fn = 40;

width = 47;
height = 34;
depth = 74;
wall = 3;

drillhole = 2.5;
gap = 10-10*$t;

box();
//side(width, height, wall);
//bottom(width, height, depth, wall);
//top(width,height,depth,wall);

module box(){
translate([wall, 0, wall])rotate([90, 0, 0])side(width, height, wall);
translate([width+wall, depth+2*gap, wall])rotate([90,0,180])side(width, height, wall);

translate([0,  gap, 0])bottom(width, height, depth, wall);

translate([width+2*wall,gap,height+gap+2*wall])rotate([0,180,0])top(width,height,depth,wall);

}
module side(w, h, wl) {
    
      difference(){
    hull() {
    translate([0, 0, 0])cylinder(r = wl, h = wl);
    translate([w, 0, 0])cylinder(r = wl, h = wl);
    translate([w, h, 0])cylinder(r = wl, h = wl);
    translate([0, h, 0])cylinder(r = wl, h = wl);
  }
      translate([0, 0, wl-1])cylinder(d1=3,d2=5, h = 1.01);
    translate([w, 0, wl-1])cylinder(d1=3,d2=5, h = 1.01);
    translate([w, h, wl-1])cylinder(d1=3,d2=5, h = 1.01);
    translate([0, h, wl-1])cylinder(d1=3,d2=5, h = 1.01);
  
        translate([0, 0, -0.01])cylinder(d=3, h = wl);
    translate([w, 0, -0.01])cylinder(d=3, h = wl);
    translate([w, h, -0.01])cylinder(d=3, h = wl);
    translate([0, h, -0.01])cylinder(d=3, h = wl);
  translate([19,4,-3])neutrik();
  }

}

module neutrik(){
    hull(){
        translate([0,0,0])cylinder(d=2,h=2);
        translate([24,29,0])cylinder(d=2,h=2);
        translate([24,0,0])cylinder(d=2,h=2);
        translate([0,29,0])cylinder(d=2,h=2);
    }
    translate([12,14.5,2])cylinder(d=24,h=5);
    translate([2.5,26.5,2])cylinder(d=3.2,h=5);
    translate([2.5+19,26.5-24,2])cylinder(d=3.2,h=5);
}

module bottom (w, h, d, wl) {
  difference() {
    union() {
      difference() {
        union() {
          translate([wl, 0, 0])cube([w, d, h / 2]);
          translate([0, 0, wl])cube([w + 2 * wl, d, h / 2]);
        }
        translate([wl, -0.01, wl])cube([w, d + 0.02, h]);
    }
      translate([wl, 0, wl])rotate([-90, 0, 0])cylinder(r = wl, h = d);
      translate([w + wl, 0, wl])rotate([-90, 0, 0])cylinder(r = wl, h = d);
    }
    translate([wl, 0, wl])rotate([-90, 0, 0])cylinder(r = drillhole/2, h = d);
    translate([w + wl, 0, wl])rotate([-90, 0, 0])cylinder(r = drillhole/2, h = d);
  }
  translate([wl-1,0,wl+h/2])cube([1,d,1]);
  translate([w+wl,0,wl+h/2])cube([1,d,1]);
}


module top (w, h, d, wl) {
  difference() {
    union() {
      difference() {
        union() {
          translate([wl, 0, 0])cube([w, d, h / 2]);
          translate([0, 0, wl])cube([w + 2 * wl, d, h / 2]);
        }
        translate([wl, -0.01, wl])cube([w, d + 0.02, h]);
    }
      translate([wl, 0, wl])rotate([-90, 0, 0])cylinder(r = wl, h = d);
      translate([w + wl, 0, wl])rotate([-90, 0, 0])cylinder(r = wl, h = d);
    }
    translate([wl, 0, wl])rotate([-90, 0, 0])cylinder(r = drillhole/2, h = d);
    translate([w + wl, 0, wl])rotate([-90, 0, 0])cylinder(r = drillhole/2, h = d);

    translate([wl-1,-0.1,wl+h/2-1])cube([1.1,d+0.2,1.1]);
    translate([w+wl-0.05,-0.1,wl+h/2-1-0.05])cube([1.1,d+0.2,1.1]);
    }

}