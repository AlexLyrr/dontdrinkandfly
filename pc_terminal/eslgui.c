//*****************************
//*GUI
//*@Author Georgios Giannakaras
//*
//*****************************

#include "pc_terminal.h"

//@Author Georgios Giannakaras
void calculateBatteryStatus(float battery)
{
		char guiText[20];
		float temp_battery, fraction;
		float battery_range = BATTERY_MAX - BATTERY_MIN;
		int battery_final;

		temp_battery = (battery - BATTERY_MIN);
		battery_range = battery_range / 100;
		temp_battery = temp_battery / battery_range;
		battery_final = (int) temp_battery;
		fraction = temp_battery / 100;
		if (battery_final < 0)
		{
			battery_final = 0;
			fraction = 0;
		}
		else if(battery_final > 100){
			battery_final = 100;
			fraction = 1;
		}
		gtk_progress_bar_set_fraction (widg.pb[0], fraction);
		sprintf(guiText, "%d%%", battery_final);
		gtk_progress_bar_set_text (widg.pb[0], guiText);
}

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
	if (pcStateGui->liftValue <=1000){
		pcStateGui->liftValue +=10;
		pcStateGui->aPressed = true;
 	}
}

G_MODULE_EXPORT void on_button_down_clicked(GtkButton *button, Widgets *widg)
{	
	if (pcStateGui->liftValue >10){
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