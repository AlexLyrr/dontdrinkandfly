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

G_MODULE_EXPORT void on_button_up_rpm_clicked(GtkButton *button, Widgets *widg)
{
	if (pcStateGui->liftValue < 1000){
		pcStateGui->liftValue +=10;
		pcStateGui->aPressed = true;
 	}
}

G_MODULE_EXPORT void on_button_down_rpm_clicked(GtkButton *button, Widgets *widg)
{	
	if (pcStateGui->liftValue >= 10){
		pcStateGui->zPressed = true;
		pcStateGui->liftValue -=10;
	}
}

G_MODULE_EXPORT void on_button_up_p_clicked(GtkButton *button, Widgets *widg)
{
	if (pcState->PValue < 1000 && (pcState->mode == 4 || pcState->mode == 5 || pcState->mode == 7 || pcState->mode == 8 || pcState->mode == 6)){
		pcState->uPressed = true;
		pcState->PValue += 1;
	}
}

G_MODULE_EXPORT void on_button_down_p_clicked(GtkButton *button, Widgets *widg)
{	
	if (pcState->PValue > 0 && (pcState->mode == 4 || pcState->mode == 5 || pcState->mode == 7 || pcState->mode == 8 || pcState->mode == 6)){
		pcState->jPressed = true;
		pcState->PValue -= 1;
	}
}

G_MODULE_EXPORT void on_button_up_p1_clicked(GtkButton *button, Widgets *widg)
{
	if (pcState->P1Value < 1000 && (pcState->mode == 5 || pcState->mode == 7 || pcState->mode == 8 || pcState->mode == 6 || pcState->mode == 9 )){
		pcState->iPressed = true;
		pcState->P1Value += 1;
	}
}

G_MODULE_EXPORT void on_button_down_p1_clicked(GtkButton *button, Widgets *widg)
{	
	if (pcState->P1Value > 0 && (pcState->mode == 5 || pcState->mode == 7 || pcState->mode == 8 || pcState->mode == 6 || pcState->mode == 9 )){
		pcState->kkPressed = true;
		pcState->P1Value -= 1;
	}
}

G_MODULE_EXPORT void on_button_up_p2_clicked(GtkButton *button, Widgets *widg)
{
	if (pcState->P2Value < 1000 && (pcState->mode == 5 || pcState->mode == 7 || pcState->mode == 8 || pcState->mode == 6 || pcState->mode == 9)){
		pcState->oPressed = true;
		pcState->P2Value += 1;
	}
}

G_MODULE_EXPORT void on_button_down_p2_clicked(GtkButton *button, Widgets *widg)
{	
	if (pcState->P2Value > 0 && (pcState->mode == 5 || pcState->mode == 7 || pcState->mode == 8  || pcState->mode == 6 || pcState->mode == 9)){
		pcState->lPressed = true;
		pcState->P2Value -= 1;
	}
}


G_MODULE_EXPORT void on_button_up_ph_clicked(GtkButton *button, Widgets *widg)
{
	if (pcState->PheightValue < 1000 && (pcState->mode == 5 || pcState->mode == 7 || pcState->mode == 8 || pcState->mode == 7)){
		pcState->yPressed = true;
		pcState->PheightValue += 1;
	}
}

G_MODULE_EXPORT void on_button_down_ph_clicked(GtkButton *button, Widgets *widg)
{	
	if (pcState->PheightValue > 0 && (pcState->mode == 5 || pcState->mode == 7 || pcState->mode == 8 || pcState->mode == 7)){
		pcState->hPressed = true;
		pcState->PheightValue -= 1;
	}
}

G_MODULE_EXPORT void on_button_up_psaz_clicked(GtkButton *button, Widgets *widg)
{
	if (pcState->PheightValue2 < 256 && (pcState->mode == 5 || pcState->mode == 7 || pcState->mode == 8 || pcState->mode == 7)){
		pcState->tPressed = true;
		pcState->PheightValue2 += 1;
	}
}

G_MODULE_EXPORT void on_button_down_psaz_clicked(GtkButton *button, Widgets *widg)
{	
	if (pcState->PheightValue2 > 0 && (pcState->mode == 5 || pcState->mode == 7 || pcState->mode == 8 || pcState->mode == 7)){
		pcState->gPressed = true;
		pcState->PheightValue2 -= 1;
	}
}

G_MODULE_EXPORT void on_button_left_roll_clicked(GtkButton *button, Widgets *widg)
{	
	if (pcState->rollValue < 180 && (pcState->mode == 2 || pcState->mode == 5 || pcState->mode == 7 || pcState->mode == 8 || pcState->mode == 6 || pcState->mode == 9)){
		pcState->leftPressed = true;
		pcState->rollValue += 1;
	}
}

G_MODULE_EXPORT void on_button_right_roll_clicked(GtkButton *button, Widgets *widg)
{	
	if (pcState->rollValue > 0 && (pcState->mode == 2 || pcState->mode == 5 || pcState->mode == 7 || pcState->mode == 8 || pcState->mode == 6 || pcState->mode == 9)){
		pcState->rightPressed = true;
		pcState->rollValue -= 1;
	}
}

G_MODULE_EXPORT void on_button_left_yaw_clicked(GtkButton *button, Widgets *widg)
{	
	if (pcState->yawValue >= 10 && (pcState->mode == 2 || pcState->mode == 4 || pcState->mode == 5 || pcState->mode == 7 || pcState->mode == 8 || pcState->mode == 6)){
		pcState->qPressed = true;
		pcState->yawValue -= 1;
	}
}

G_MODULE_EXPORT void on_button_right_yaw_clicked(GtkButton *button, Widgets *widg)
{	
	if (pcState->yawValue < 180 && (pcState->mode == 2 || pcState->mode == 4 || pcState->mode == 5 || pcState->mode == 7 || pcState->mode == 8 || pcState->mode == 6)){
		pcState->wPressed = true;
		pcState->yawValue += 1;
	}
}

G_MODULE_EXPORT void on_button_down_pitch_clicked(GtkButton *button, Widgets *widg)
{	
	if (pcState->pitchValue < 180 && (pcState->mode == 2 || pcState->mode == 5 || pcState->mode == 7 || pcState->mode == 8 || pcState->mode == 6  || pcState->mode == 9)){
		pcState->downPressed = true;
		pcState->pitchValue += 1;
	}
}

G_MODULE_EXPORT void on_button_up_pitch_clicked(GtkButton *button, Widgets *widg)
{	
	if (pcState->pitchValue > 0 && (pcState->mode == 2 || pcState->mode == 5 || pcState->mode == 7 || pcState->mode == 8 || pcState->mode == 6  || pcState->mode == 9)){
		pcState->upPressed = true;
		pcState->pitchValue -= 1;
	}
}

G_MODULE_EXPORT void help_about(GtkWidget *widget, gpointer data){

    builder2 = gtk_builder_new();
        
    gtk_builder_add_from_file( builder2, "eslgui.glade", NULL );

    dialog  = GTK_WIDGET( gtk_builder_get_object( builder2, "aboutdialog1" ) );
    gtk_window_set_transient_for (GTK_WINDOW(dialog), data);

    gtk_widget_show (dialog);
    response = gtk_dialog_run(GTK_DIALOG(dialog));

    gtk_widget_destroy(dialog);

    g_object_unref(G_OBJECT(builder2));
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
    widg.l[21] = GTK_LABEL( gtk_builder_get_object( builder, "label_p" ) );
    widg.l[22] = GTK_LABEL( gtk_builder_get_object( builder, "label_p1" ) );
    widg.l[23] = GTK_LABEL( gtk_builder_get_object( builder, "label_p2" ) );
    widg.l[24] = GTK_LABEL( gtk_builder_get_object( builder, "label_ph" ) );
    widg.l[25] = GTK_LABEL( gtk_builder_get_object( builder, "label_psaz" ) );
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
	sprintf(guiText, "%hhu", pcStateGui->PValue);
	gtk_label_set_label(widg.l[21], guiText);
	sprintf(guiText, "%hhu", pcStateGui->P1Value);
	gtk_label_set_label(widg.l[22], guiText);
	sprintf(guiText, "%hhu", pcStateGui->P2Value);
	gtk_label_set_label(widg.l[23], guiText);
	sprintf(guiText, "%hhu", pcStateGui->PheightValue);
	gtk_label_set_label(widg.l[24], guiText);
	sprintf(guiText, "%hhu", pcStateGui->PheightValue2);
	gtk_label_set_label(widg.l[25], guiText);
    gtk_main();

    return NULL;
}