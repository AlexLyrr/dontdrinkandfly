//*****************************
//*GUI
//*@Author Georgios Giannakaras
//*
//*****************************
#include <glib.h>
#include <gtk/gtk.h>
#include "pc_terminal.h"

//For all the below G_MODULE_EXPORT functions @Author Georgios Giannakaras
//
G_MODULE_EXPORT void on_button_safe_clicked(GtkButton *button, Widgets *widg)
{
	pcStateGui->n0Pressed = true;
	pcStateGui->mode = 0;
}

G_MODULE_EXPORT void on_button_panic_clicked(GtkButton *button, Widgets *widg)
{
	pcStateGui->n1Pressed = true;
	pcStateGui->mode = 1;
}

G_MODULE_EXPORT void on_button_manual_clicked(GtkButton *button, Widgets *widg)
{
	pcStateGui->n2Pressed = true;
	pcStateGui->mode = 2;
}

G_MODULE_EXPORT void on_button_calibration_clicked(GtkButton *button, Widgets *widg)
{
	pcStateGui->n3Pressed = true;
	pcStateGui->mode = 3;
}

G_MODULE_EXPORT void on_button_yaw_clicked(GtkButton *button, Widgets *widg)
{
	pcStateGui->n4Pressed = true;
	pcStateGui->mode = 4;
}

G_MODULE_EXPORT void on_button_fullControl_clicked(GtkButton *button, Widgets *widg)
{
	pcStateGui->n5Pressed = true;
	pcStateGui->mode = 5;
}

G_MODULE_EXPORT void on_button_raw_clicked(GtkButton *button, Widgets *widg)
{
	pcStateGui->n6Pressed = true;
	pcStateGui->mode = 6;
}

G_MODULE_EXPORT void on_button_height_clicked(GtkButton *button, Widgets *widg)
{
	pcStateGui->n7Pressed = true;
	pcStateGui->mode = 7;
}

G_MODULE_EXPORT void on_button_wireless_clicked(GtkButton *button, Widgets *widg)
{
	pcStateGui->n8Pressed = true;
	pcStateGui->mode = 8;
}

G_MODULE_EXPORT void on_button_abort_clicked(GtkButton *button, Widgets *widg)
{
	pcStateGui->escPressed = true;
}

G_MODULE_EXPORT void on_button_up_clicked(GtkButton *button, Widgets *widg)
{
	if (pcStateGui->liftValue < 1000){
		pcStateGui->liftValue +=10;
		pcStateGui->aPressed = true;
 	}
}

G_MODULE_EXPORT void on_button_down_clicked(GtkButton *button, Widgets *widg)
{	
	if (pcStateGui->liftValue >= 10){
		pcStateGui->zPressed = true;
		pcStateGui->liftValue -=10;
	}
}

//@Author Georgios Giannakaras
void *guiThread(void *vargp){
	char guiText[20];

    gtk_init(NULL, NULL);

    builder = gtk_builder_new();
        
    gtk_builder_add_from_file( builder, "eslgui.glade", NULL );

    window  = GTK_WIDGET( gtk_builder_get_object( builder, "window_main" ) );

    gtk_window_set_title(GTK_WINDOW(window), "Dont drink and fly");

    widg.l[0] = GTK_LABEL( gtk_builder_get_object( builder, "label_motor1" ) );
    widg.l[1] = GTK_LABEL( gtk_builder_get_object( builder, "label_motor2" ) );
    widg.l[2] = GTK_LABEL( gtk_builder_get_object( builder, "label_motor3" ) );
    widg.l[3] = GTK_LABEL( gtk_builder_get_object( builder, "label_motor4" ) );
    widg.l[4] = GTK_LABEL( gtk_builder_get_object( builder, "label_mode" ) );
    widg.l[5] = GTK_LABEL( gtk_builder_get_object( builder, "label_battery" ) );
    widg.l[6] = GTK_LABEL( gtk_builder_get_object( builder, "label_roll_drone" ) );
    widg.l[7] = GTK_LABEL( gtk_builder_get_object( builder, "label_pitch_drone" ) );
    widg.l[8] = GTK_LABEL( gtk_builder_get_object( builder, "label_height_drone" ) );
    widg.l[9] = GTK_LABEL( gtk_builder_get_object( builder, "label_roll_pc" ) );
    widg.l[10] = GTK_LABEL( gtk_builder_get_object( builder, "label_pitch_pc" ) );
    widg.l[11] = GTK_LABEL( gtk_builder_get_object( builder, "label_yaw_pc" ) );
    widg.l[12] = GTK_LABEL( gtk_builder_get_object( builder, "label_lift_pc" ) );
    widg.l[13] = GTK_LABEL( gtk_builder_get_object( builder, "label_roll_keyboard" ) );
    widg.l[14] = GTK_LABEL( gtk_builder_get_object( builder, "label_pitch_keyboard" ) );
    widg.l[15] = GTK_LABEL( gtk_builder_get_object( builder, "label_yaw_keyboard" ) );
    widg.l[16] = GTK_LABEL( gtk_builder_get_object( builder, "label_lift_keyboard" ) );
    widg.l[17] = GTK_LABEL( gtk_builder_get_object( builder, "label_roll_joystick" ) );
    widg.l[18] = GTK_LABEL( gtk_builder_get_object( builder, "label_pitch_joystick" ) );
    widg.l[19] = GTK_LABEL( gtk_builder_get_object( builder, "label_yaw_joystick" ) );
    widg.l[20] = GTK_LABEL( gtk_builder_get_object( builder, "label_lift_joystick" ) );
    widg.lb[0] = GTK_LEVEL_BAR( gtk_builder_get_object( builder, "levelbar_motor1" ) );
    widg.lb[1] = GTK_LEVEL_BAR( gtk_builder_get_object( builder, "levelbar_motor2" ) );
    widg.lb[2] = GTK_LEVEL_BAR( gtk_builder_get_object( builder, "levelbar_motor3" ) );
    widg.lb[3] = GTK_LEVEL_BAR( gtk_builder_get_object( builder, "levelbar_motor4" ) );
    widg.pb[0] = GTK_PROGRESS_BAR( gtk_builder_get_object( builder, "progressbar_battery" ) );

    gtk_builder_connect_signals( builder, &widg );
        
    g_object_unref( G_OBJECT( builder ) );

    gtk_widget_show (window);

    //Initialize gui labels
	for (int i = 0; i < 4; ++i)
	{
		sprintf(guiText, "%hu RPM", 0);
		gtk_label_set_label(widg.l[i], guiText);
		gtk_level_bar_set_value (widg.lb[i], 0);
	}
	sprintf(guiText, "Safe");
	gtk_label_set_label(widg.l[4], guiText);
	sprintf(guiText, "%hhu", 0);
	gtk_label_set_label(widg.l[8], guiText);
    gtk_main();

    return NULL;
}