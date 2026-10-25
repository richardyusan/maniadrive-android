#!/usr/bin/python
# This a Python version of a Raydium's tutorial:
# http://wiki.raydium.org/wiki/TutorielUtiliserLaPhysique (fr)
# http://wiki.raydium.org/wiki/TutorialUsePhysics (bad en)

# See gen_bindings.php to generate bindings and copy rayphp/ in this
# directory to test this script.

import raydium
import sys


def create_car():
    BREAK_FORCE=130
    ROTFRICTION=0.0005
    ERP=0.3
    CFM=0.8
     
    raydium.raydium_ode_object_delete_name("WATURE");
     
    a=raydium.raydium_ode_object_create("WATURE");
    raydium.raydium_ode_object_box_add("corps",a,1,1.2,0.6,0.4,raydium.RAYDIUM_ODE_STANDARD,0,"clio.tri");
    raydium.raydium_ode_element_slip_name("corps",raydium.RAYDIUM_ODE_SLIP_ICE);

    raydium.raydium_ode_object_sphere_add("pneu_ag",a,0.5,raydium.RAYDIUM_ODE_AUTODETECT,raydium.RAYDIUM_ODE_STANDARD,0,"roue5.tri");
    raydium.raydium_ode_element_rotfriction_name("pneu_ag",ROTFRICTION);
    raydium.raydium_ode_element_move_name_3f("pneu_ag",0.42,0.253,-0.180);
    raydium.raydium_ode_joint_attach_hinge2_name("suspet_ag","corps","pneu_ag",0,0,1,0,1,0);
    raydium.raydium_ode_joint_break_force_name("suspet_ag",BREAK_FORCE);
    raydium.raydium_ode_joint_suspension_name("suspet_ag",ERP,CFM);

    raydium.raydium_ode_object_sphere_add("pneu_ad",a,0.5,raydium.RAYDIUM_ODE_AUTODETECT,raydium.RAYDIUM_ODE_STANDARD,0,"roue5.tri");
    raydium.raydium_ode_element_rotfriction_name("pneu_ad",ROTFRICTION);
    raydium.raydium_ode_element_move_name_3f("pneu_ad",0.42,-0.253,-0.180);
    raydium.raydium_ode_joint_attach_hinge2_name("suspet_ad","corps","pneu_ad",0,0,1,0,1,0);
    raydium.raydium_ode_joint_break_force_name("suspet_ad",BREAK_FORCE);
    raydium.raydium_ode_joint_suspension_name("suspet_ad",ERP,CFM);

    raydium.raydium_ode_object_sphere_add("pneu_rg",a,0.5,raydium.RAYDIUM_ODE_AUTODETECT,raydium.RAYDIUM_ODE_STANDARD,0,"roue5.tri");
    raydium.raydium_ode_element_rotfriction_name("pneu_rg",ROTFRICTION);
    raydium.raydium_ode_element_move_name_3f("pneu_rg",-0.444,0.253,-0.180);
    raydium.raydium_ode_joint_attach_hinge2_name("suspet_rg","corps","pneu_rg",0,0,1,0,1,0);
    raydium.raydium_ode_joint_break_force_name("suspet_rg",BREAK_FORCE);
    raydium.raydium_ode_joint_suspension_name("suspet_rg",ERP,CFM);
    raydium.raydium_ode_joint_hinge2_block_name("suspet_rg",1);

    raydium.raydium_ode_object_sphere_add("pneu_rd",a,0.5,raydium.RAYDIUM_ODE_AUTODETECT,raydium.RAYDIUM_ODE_STANDARD,0,"roue5.tri");
    raydium.raydium_ode_element_rotfriction_name("pneu_rd",ROTFRICTION);
    raydium.raydium_ode_element_move_name_3f("pneu_rd",-0.444,-0.253,-0.180);
    raydium.raydium_ode_joint_attach_hinge2_name("suspet_rd","corps","pneu_rd",0,0,1,0,1,0);
    raydium.raydium_ode_joint_break_force_name("suspet_rd",BREAK_FORCE);
    raydium.raydium_ode_joint_suspension_name("suspet_rd",ERP,CFM);
    raydium.raydium_ode_joint_hinge2_block_name("suspet_rd",1);

    raydium.raydium_ode_motor_create("moteur",a,raydium.RAYDIUM_ODE_MOTOR_ENGINE);
    raydium.raydium_ode_motor_attach_name("moteur","suspet_ag",1);
    raydium.raydium_ode_motor_attach_name("moteur","suspet_ad",1);
    raydium.raydium_ode_motor_power_max_name("moteur",0.2);
 
    raydium.raydium_ode_motor_create("direction",a,raydium.RAYDIUM_ODE_MOTOR_ANGULAR);
    raydium.raydium_ode_motor_attach_name("direction","suspet_ag",0);
    raydium.raydium_ode_motor_attach_name("direction","suspet_ad",0);
    raydium.raydium_ode_motor_power_max_name("direction",0.2);


def display():
    raydium.raydium_joy_key_emul();
    
    direct=raydium.cvar.raydium_joy_x*0.3;
    speed=raydium.cvar.raydium_joy_y*55;

    raydium.raydium_ode_motor_speed_name("moteur",-speed);
    raydium.raydium_ode_motor_angle_name("direction",direct);

    if raydium.cvar.raydium_key_last==1027:
	raydium.exit(0);

    raydium.raydium_clear_frame();
    raydium.raydium_ode_element_camera_inboard_name("corps",0.2,0,0.1,2,0,0);
 
    raydium.raydium_ode_draw_all(0);

    if raydium.raydium_key_pressed(1):
	raydium.raydium_ode_draw_all(1);
 
    raydium.raydium_rendering_finish();


raydium.raydium_init_args(sys.argv);
raydium.raydium_window_create(640,480,raydium.RAYDIUM_RENDERING_WINDOW,"Physics test");
raydium.raydium_texture_filter_change(raydium.RAYDIUM_TEXTURE_FILTER_TRILINEAR);
raydium.cvar.raydium_projection_near=0.01;
raydium.cvar.raydium_projection_far=1000;
raydium.cvar.raydium_projection_fov=60;
raydium.raydium_window_view_update();
raydium.raydium_light_on(0);
raydium.raydium_light_conf_7f(0,50,150,200,1000000,1,0.9,0.5)
raydium.raydium_background_color_change(1,0.9,0.5,1)
raydium.raydium_fog_disable();
raydium.raydium_ode_ground_set_name("cocorobix.tri");
create_car();

raydium.SetMainLoopPython(display);
