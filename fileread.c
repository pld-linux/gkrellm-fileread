/* GKrellM
|  Copyright (C) 1999-2000 Bill Wilson
|
|  Author:  Bill Wilson    bill@gkrellm.net
|  Latest versions might be found at:  http://gkrellm.net
|
|  This program is free software which I release under the GNU General Public
|  License. You may redistribute and/or modify this program under the terms
|  of that license as published by the Free Software Foundation; either
|  version 2 of the License, or (at your option) any later version.
|
|  This program is distributed in the hope that it will be useful,
|  but WITHOUT ANY WARRANTY; without even the implied warranty of
|  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
|  GNU General Public License for more details.
|
|  To get a copy of the GNU General Puplic License,  write to the
|  Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/* Fileread plugin for GKrellM
|
|  Copyright (C) 2000  Henry Palonen
|
|  Author:  Henry Palonen		h@yty.net
|
|  Version: for gkrellm version 0.10.4 and up
| 
|  Changed to support any number of files and more data types by 
|   Thomas Möstl <tmoestl@gmx.net> 2000-09.
*/

/* Simple plugin for reading a value from a file.
|  File should contain one line with label:
|
|  Outside: +23.43
|  
|  That is simply read to monitor and used there.
|
*/

/*
|  Plugin is based to Bill Wilson's great work, GKrellM & meminfo.c & template.c files.
|
|  Latest versions might be found from http://www.yty.net/h/gkrellm
*/

/* If this file is compiled as a shared object file and installed in
|  in your ~/.gkrellm/plugins directory, then it will be loaded when
|  gkrellm starts up.
|
|  Now you can just type "make" in the directory and it should compile nicely, putting out "fileread.so" 
|  - file (Thanks to Thomas for suplying the Makefile)
*/ 

/*
|  Starting code, finally ;)
*/

#include <gkrellm/gkrellm.h>
#include <utime.h>
#include <dirent.h>

#define TYPE_FLOAT  0
#define TYPE_INT    1
#define TYPE_STRING 2
#define TYPE_NUM    3

gchar *types[3]={"float","integer","string"};

typedef struct
        {
	gchar    *path;
	gchar    *name;
	gint     type;
	Panel	 *fileread;
        Decal	 *decal_fileread;
        } item_t;

#define PIPE_SIZE	4

static GArray   *fileread_file_paths;
static gint     fileread_file_path_num;
static gint     ctrl_path_num;
GtkWidget       *fileread_vbox;

void setup_fileread();
void destroy_fileread();

/*
 *	Drawing actual text-string containing label and value
 */

static void
draw_fileread(gchar temp[20], gint i)
        {
     		Style           *style;
	        TextStyle       *ts;

	        style = gkrellm_meter_style(DEFAULT_STYLE/*gkrellm_lookup_meter_style_id(UPTIME_STYLE_NAME)*/);
	        ts = gkrellm_meter_textstyle(DEFAULT_STYLE/*gkrellm_lookup_meter_style_id(UPTIME_STYLE_NAME)*/);
		g_array_index(fileread_file_paths,item_t, i).decal_fileread->text_style=*ts;

		g_array_index(fileread_file_paths,item_t, i).decal_fileread->x_off =
			gkrellm_chart_width () - 
			gdk_string_width (g_array_index(fileread_file_paths,item_t, i).fileread->textstyle->font, temp) - 
			2 * g_array_index(fileread_file_paths,item_t, i).fileread->style->margin;
		gkrellm_draw_decal_text (g_array_index(fileread_file_paths,item_t, i).fileread, 
					 g_array_index(fileread_file_paths,item_t, i).decal_fileread, temp,
					 -1);

		g_array_index(fileread_file_paths,item_t, i).decal_fileread->value=0;
	        gkrellm_draw_layers(g_array_index(fileread_file_paths,item_t, i).fileread);
        }

/*
 *	Updating monitor value
 */

/*
 | label and data must point to two preallocated char[20] buffers.
 */
static void
update_fileread_vals(char *path, int type, char *label, char *data)
        {
	FILE	*f;

	gfloat  tf;
	gint    ti;

	if (GK.debug)
		printf("path: %s\n",path);
	if ((f = fopen(path, "r")) != NULL)
	        {
		if (GK.debug)
			printf("tp: %i\n",type);
		switch (type)
		        {
			case TYPE_FLOAT:
				if (fscanf(f, "%18[^:]: %f", label, &tf)!=2) 
				        {
					strcpy(label,"I/O error");
					strcpy(data,"");
				        } 
				else 
				        {
					snprintf(data,20,"%.2f",tf);
					}
				fclose(f);
				break;
			case TYPE_INT:
				if (fscanf(f, "%18[^:]: %i", label, &ti)!=2) 
				        {
					strcpy(label,"I/O error");
					strcpy(data,"");
				        }
				else 
				        {
					snprintf(data,20,"%i",ti);
					}
				fclose(f);
				break;
			case TYPE_STRING:
				if (fscanf(f, "%18[^:]: %20s", label, data)!=2) 
				        {
					strcpy(label,"I/O error");
					strcpy(data,"");
				        }
				fclose(f);
				break;
			}
		strcat(label,":");
		}
	else 
	        {
		strcpy(label,"I/O error");
		strcpy(data,"");
		}
	}

static void
update_fileread_data()
	{
        gchar   temp[20];
        gchar   templabel[20];

	gint    i;
	gint    changed=0;

	if (GK.debug)
		printf("update: %i\n",fileread_file_path_num);
	for (i=0;i<fileread_file_path_num;i++)
	        {
		update_fileread_vals(g_array_index(fileread_file_paths,item_t,i).path, 
				     g_array_index(fileread_file_paths,item_t,i).type, templabel, temp);

		if (GK.debug)
			printf("draw: %s: %s (%i)\n",templabel,temp,i);

		if (strcmp(templabel,g_array_index(fileread_file_paths,item_t,i).name)!=0) 
		        {
			g_free(g_array_index(fileread_file_paths,item_t,i).name);
			g_array_index(fileread_file_paths,item_t,i).name=g_strdup(templabel);
			changed=1;
			}
		draw_fileread(temp,i);
		}
	if (changed)
	        {
		destroy_fileread();
		setup_fileread();
		update_fileread_data();
		}
	}

static void
update_fileread()
	{

	/* It's enough to update once every ten seconds. It's actually too
	|  often. Should be minute or so. But then there should be a call which 
	|  updates status with "force" at startup so files value is readable
	|  right away. I worst case there would be one minute delay. Now delay is 10 secs max.
	|
	|  -tm, 2000-09-30: The forced update at startup is now in place.
	*/
	
	if (GK.ten_second_tick)
	        {
		update_fileread_data();
		}
	}	

static gint
fileread_expose_event (GtkWidget *widget, GdkEventExpose *event)
	{
	gint i;

	for (i=0;i<fileread_file_path_num;i++) 
	        {
		if (widget == g_array_index(fileread_file_paths,item_t, i).fileread->drawing_area)
		        {
			gdk_draw_pixmap(widget->window,
					widget->style->fg_gc[GTK_WIDGET_STATE (widget)],
					g_array_index(fileread_file_paths,item_t, i).fileread->pixmap,
					event->area.x, event->area.y,
					event->area.x, event->area.y,
					event->area.width, event->area.height);
			}

		}
	return FALSE;
	}

/*
 *	Creating fileread-monitor
 */

void 
destroy_fileread()
        {
	gint i;

	for (i=0;i<fileread_file_path_num;i++) 
	        {
		GK.monitor_height -= g_array_index(fileread_file_paths,item_t, i).fileread->h;
                gkrellm_destroy_krell_list(g_array_index(fileread_file_paths,item_t, i).fileread);
                gkrellm_destroy_decal_list(g_array_index(fileread_file_paths,item_t, i).fileread);
		gkrellm_destroy_panel(g_array_index(fileread_file_paths,item_t, i).fileread);
		g_array_index(fileread_file_paths,item_t, i).fileread=NULL;
	        }

        }

void 
setup_fileread()
        { 
        Style           *style;
	static Style	fileread_style;
        GdkImlibImage   *bg_image;
        gchar           data[20];
        gchar           label[20];

        TextStyle               *ts;
        gint                    w,i,first_create;

	#ifdef USE_FS_STYLE_WHICH_MAY_SHOW_A_DIFF_ONLY_IN_THE_DEFAULT_THEME
	#define MY_STYLE        FS_STYLE
	#else
	#define MY_STYLE        DEFAULT_STYLE
	#endif  
        
        /* Allocate a panel structure.  plug->label and plug->textstyle
        |  structures will also be allocated...
 	*/

	style = gkrellm_style_new0();
	for (i=0;i<fileread_file_path_num;i++)  
	        {
		/* Modified for 0.9.0 gkrellm */
	
		first_create=!g_array_index(fileread_file_paths,item_t, i).fileread;
		if (first_create)
		        {
			g_array_index(fileread_file_paths,item_t, i).fileread = gkrellm_panel_new0();
			}
		else
		        {
			gkrellm_destroy_krell_list(g_array_index(fileread_file_paths,item_t, i).fileread);
			gkrellm_destroy_decal_list(g_array_index(fileread_file_paths,item_t, i).fileread);
			}

		fileread_style = *gkrellm_meter_style(DEFAULT_STYLE/*gkrellm_lookup_meter_style_id(UPTIME_STYLE_NAME)*/);
		
		g_array_index(fileread_file_paths,item_t, i).fileread->textstyle = 
			gkrellm_meter_textstyle(DEFAULT_STYLE/*gkrellm_lookup_meter_style_id(UPTIME_STYLE_NAME)*/);
		ts = g_array_index(fileread_file_paths,item_t, i).fileread->textstyle;
		w = gdk_string_width(ts->font, "Outside: -23.45") + 2;
		if (w > UC.chart_width - 2 * fileread_style.margin)
			w = UC.chart_width - 2 * fileread_style.margin;
		

		g_array_index(fileread_file_paths,item_t, i).decal_fileread = 
			gkrellm_create_text_decal(g_array_index(fileread_file_paths,item_t, i).fileread, "Out: -53.24",
						  ts, &fileread_style, -1, -1, w);

		gkrellm_create_krell(g_array_index(fileread_file_paths,item_t, i).fileread, 
				     gkrellm_krell_meter_image(DEFAULT_STYLE), &fileread_style);
		
		/* Update for 0.9.0 */
		g_array_index(fileread_file_paths,item_t, i).fileread->textstyle = gkrellm_meter_textstyle(DEFAULT_STYLE);
		
		/* Initial update to get the title */
		update_fileread_vals(g_array_index(fileread_file_paths,item_t,i).path, 
				     g_array_index(fileread_file_paths,item_t,i).type, label, data);
		g_free(g_array_index(fileread_file_paths,item_t,i).name);
		g_array_index(fileread_file_paths,item_t,i).name=g_strdup(label);

		gkrellm_configure_panel(g_array_index(fileread_file_paths,item_t, i).fileread,
					g_array_index(fileread_file_paths,item_t, i).name , &fileread_style);
		
		bg_image = gkrellm_bg_meter_image(DEFAULT_STYLE);
		gkrellm_create_panel(fileread_vbox, g_array_index(fileread_file_paths,item_t, i).fileread, bg_image);
        	
		GK.monitor_height += g_array_index(fileread_file_paths,item_t, i).fileread->h;
        	
		/* Modified for 0.9.0 gkrellm */
		if (first_create)
		        {
			gtk_signal_connect(GTK_OBJECT (g_array_index(fileread_file_paths,item_t, i).fileread->drawing_area), 
					   "expose_event", (GtkSignalFunc) fileread_expose_event, NULL);
		        }
		}
	}

void
create_fileread(GtkWidget *vbox, gint first_create)
        {
	fileread_vbox=vbox;
	setup_fileread();
	update_fileread_data();
	}

/*
 *	Configuration page
 *	Configuration contains path to file ;)
 */

static GtkWidget                *fileread_path_entry;
static GtkWidget                *fileread_type_list;
static gint                     fileread_type=0;
static GtkWidget                *fileread_item_list;
static gint                     fileread_item=0;
static gchar			*fileread_info_text =
"Fileread plugin reads a file and displays \n"
"'label number' pair at monitor. This can be used  \n"
"for numerous things. I'm using it to display outside\n"
"temperature from my ds1820 temp-sensor.\n\n"
"File should contain just one line. Example:\n\n"
"Out: -23.4\n\n"
"Under GPL, Henry Palonen, 2000\n"
"http://www.yty.net/h/gkrellm/  &  h@yty.net\n\n"
"Modified by Thomas Möstl <tmoestl@gmx.net> to\n"
"support multiple data types and an arbitrary\n"
"number of files.";

/* Shamelessly ripped from the Gtk docs. */
void fr_message(gchar *message) 
        {
        GtkWidget *dialog, *label, *okay_button;
        dialog = gtk_dialog_new();
        label = gtk_label_new (message);
        okay_button = gtk_button_new_with_label("OK");
        gtk_signal_connect_object (GTK_OBJECT (okay_button), "clicked",
                                   GTK_SIGNAL_FUNC (gtk_widget_destroy), GTK_OBJECT(dialog));
        gtk_container_add (GTK_CONTAINER (GTK_DIALOG(dialog)->action_area),
                           okay_button);
        gtk_container_add (GTK_CONTAINER (GTK_DIALOG(dialog)->vbox),
                           label);
        gtk_widget_show_all (dialog);
        }


static void
type_sel(GtkWidget *clist, gint row, gint column, GdkEventButton *event, gpointer data )
        {
	fileread_type=row;
	}

static void
item_sel(GtkWidget *clist, gint row, gint column, GdkEventButton *event, gpointer data )
        {
	fileread_item=row;
	}

static void
destroy_item(gpointer data)
        {
	g_free(((item_t *)data)->path);
	g_free(data);     
	}

static void
on_add_click(GtkButton *button, gpointer *data)
        {
	gchar *txt=g_strdup(gtk_entry_get_text(GTK_ENTRY(fileread_path_entry)));
	gchar *list[2];
	item_t *item;
	
	if (!txt[0]) 
	        {
		fr_message("Please enter a pathname!");
		g_free(txt);
		return;
	        } 
	else
		{
        	if (fileread_type<0) 
	                {
			fr_message("Please select a data type!");
			g_free(txt);
			return;
			} 
		else
		        {
			item=g_new(item_t,1);
			item->path=txt;
			item->type=fileread_type;
			list[0]=txt;
			list[1]=types[item->type];
			gtk_clist_set_row_data_full(GTK_CLIST(fileread_item_list),
						    gtk_clist_append(GTK_CLIST(fileread_item_list),list),
						    (gpointer)item, destroy_item);
			gtk_clist_columns_autosize(GTK_CLIST(fileread_item_list));
			ctrl_path_num++;
			}
		}
	}

static void
on_del_click(GtkButton *button, gpointer *data)
        {
	if (gtk_clist_get_row_data(GTK_CLIST(fileread_item_list),fileread_item)) 
	        {
		gtk_clist_remove(GTK_CLIST(fileread_item_list),fileread_item);
		ctrl_path_num--;
	        }
	}

static void
create_fileread_tab(GtkWidget *tab_vbox)
        {
	GtkWidget		*vbox;
	GtkWidget		*tabs;
        GtkWidget               *text;
        GtkWidget               *scrolled;
	GtkWidget               *btn_add;
	GtkWidget               *btn_del;
	GtkWidget               *btn_hbox;
	GtkWidget               *data_hbox;
	GtkWidget               *path_lbl;
	GtkWidget               *type_lbl;
	gchar                   *titles[2]={"File","Type"};
	gchar                   *tmp[2];
	gint                    i;
	item_t                  *item;

        tabs = gtk_notebook_new();
        gtk_notebook_set_tab_pos(GTK_NOTEBOOK(tabs), GTK_POS_TOP);
        gtk_box_pack_start(GTK_BOX(tab_vbox), tabs, TRUE, TRUE, 0);

/* File location tab */

        vbox = create_tab(tabs, "File locations");
	data_hbox = gtk_hbox_new(FALSE, 5);
	btn_hbox = gtk_hbox_new(FALSE, 5);
        fileread_path_entry = gtk_entry_new_with_max_length(255);
	btn_add=gtk_button_new_with_label("Add");
	gtk_signal_connect(GTK_OBJECT(btn_add), "clicked",
			   GTK_SIGNAL_FUNC(on_add_click),NULL);
	btn_del=gtk_button_new_with_label("Delete");
	gtk_signal_connect(GTK_OBJECT(btn_del), "clicked",
			   GTK_SIGNAL_FUNC(on_del_click),NULL);

	fileread_type=-1;
	fileread_type_list=gtk_clist_new(1);
	for (i=0;i<TYPE_NUM;i++)
	        {
		gtk_clist_append(GTK_CLIST(fileread_type_list),&types[i]);
		}
	gtk_signal_connect(GTK_OBJECT(fileread_type_list), "select-row",
			   GTK_SIGNAL_FUNC(type_sel),
			   NULL);

	gtk_clist_set_selection_mode(GTK_CLIST(fileread_type_list),GTK_SELECTION_BROWSE);

	path_lbl=gtk_label_new("File:");
	type_lbl=gtk_label_new("Data type:");

	fileread_item_list=gtk_clist_new_with_titles(2,titles);
	gtk_signal_connect(GTK_OBJECT(fileread_item_list), "select-row",
			   GTK_SIGNAL_FUNC(item_sel),
			   NULL);
	gtk_clist_set_selection_mode(GTK_CLIST(fileread_item_list),GTK_SELECTION_SINGLE);

	ctrl_path_num=fileread_file_path_num;
	for (i=0;i<ctrl_path_num;i++)
	        {
		tmp[0]=g_array_index(fileread_file_paths,item_t,i).path;
		tmp[1]=types[g_array_index(fileread_file_paths,item_t,i).type];
		item=g_new(item_t,1);
		item->path=g_strdup(g_array_index(fileread_file_paths,item_t,i).path);
		item->type=g_array_index(fileread_file_paths,item_t,i).type;
		gtk_clist_set_row_data_full(GTK_CLIST(fileread_item_list),
					    gtk_clist_append(GTK_CLIST(fileread_item_list),tmp),
					    (gpointer)item, destroy_item);
		}
	fileread_item=0;
	if (ctrl_path_num==0)
		fileread_item=-1;
	gtk_clist_columns_autosize(GTK_CLIST(fileread_item_list));

        gtk_box_pack_start(GTK_BOX(data_hbox), path_lbl, TRUE, FALSE, 2);
        gtk_box_pack_start(GTK_BOX(data_hbox), fileread_path_entry, TRUE, FALSE, 2);
        gtk_box_pack_start(GTK_BOX(data_hbox), type_lbl, TRUE, FALSE, 2);
        gtk_box_pack_start(GTK_BOX(data_hbox), fileread_type_list, TRUE, TRUE, 2);

        gtk_box_pack_start(GTK_BOX(btn_hbox), btn_add, TRUE, TRUE, 2);
        gtk_box_pack_start(GTK_BOX(btn_hbox), btn_del, TRUE, TRUE, 2);

        gtk_box_pack_start(GTK_BOX(vbox), data_hbox, FALSE, FALSE, 2);
        gtk_box_pack_start(GTK_BOX(vbox), btn_hbox, FALSE, FALSE, 2);
        gtk_box_pack_start(GTK_BOX(vbox), fileread_item_list, TRUE, TRUE, 2);

/* Info tab */

        vbox = create_tab(tabs, "Info");
        scrolled = gtk_scrolled_window_new(NULL, NULL);
        gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled),
                        GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
        gtk_box_pack_start(GTK_BOX(vbox), scrolled, TRUE, TRUE, 0);
        text = gtk_text_new(NULL, NULL);
        gtk_text_insert(GTK_TEXT(text), NULL, NULL, NULL, fileread_info_text, -1);
        gtk_text_set_editable(GTK_TEXT(text), FALSE);
        gtk_container_add(GTK_CONTAINER(scrolled), text);

        }

/*
 *	Saving config
 */

static void
save_fileread_config(FILE *f)
        {
	gint i;
	fprintf(f,"fileread num %i\n",fileread_file_path_num);
	for (i=0;i<fileread_file_path_num;i++)
	        {
		fprintf(f,"fileread filename%i %s\n",i,g_array_index(fileread_file_paths,item_t,i).path);
		fprintf(f,"fileread type%i %i\n",i,g_array_index(fileread_file_paths,item_t,i).type);
	        }
        }

/*
 *  Loading config
 */

static item_t  c;

static void
load_fileread_config(gchar *arg)
        {
        gchar   item[256];
        gint    n,i,j;

	if (GK.debug)
		printf("fileread_config=<%s>\n", arg);
        n = sscanf(arg, "num %i",&i);
	if (n==1) 
	        {
		fileread_file_path_num=i; 
                } 
        else 
                {
                n = sscanf(arg, "filename%i %s", &i,item);
	        if (n==2) 
		        {
			c.path=g_strdup(item);
	                }
		else
		        {
			n = sscanf(arg, "type%i %i", &i, &j);
			if (n==2) 
				{
				c.type=j;
				c.fileread=NULL;
				c.name=g_strdup("");
				c.decal_fileread=NULL;
				g_array_append_val(fileread_file_paths,c);
				}
			}
		} 
	}

/*
 *	Applying config.
 *	This includes only updating fileread_file_path variable.
 */

static void
apply_fileread_config()
	{
	gint                   i;
	item_t                 item; 

	destroy_fileread();
	
	for (i=0;i<fileread_file_path_num;i++) 
	        {
		g_free(g_array_index(fileread_file_paths,item_t, i).path);
		g_free(g_array_index(fileread_file_paths,item_t, i).name);
		}
	fileread_file_path_num=ctrl_path_num;
	g_array_free(fileread_file_paths,TRUE);
	fileread_file_paths=g_array_new(FALSE,TRUE,sizeof(item_t));
	for (i=0;i<ctrl_path_num;i++) 
	        {
		item.path=g_strdup(((item_t *)gtk_clist_get_row_data(GTK_CLIST(fileread_item_list),i))->path);
		item.type=((item_t *)gtk_clist_get_row_data(GTK_CLIST(fileread_item_list),i))->type;
		item.fileread=NULL;
		item.decal_fileread=NULL;
		item.name=g_strdup("");
		g_array_append_val(fileread_file_paths,item);
	        }
	setup_fileread();
	update_fileread_data();
	}


static Monitor  plugin_mon      =
        {
        "Fileread",               	    /* Name, for config tab.    */
        0,                                  /* Id,  0 if a plugin       */
        create_fileread,          	    /* The create function      */
        update_fileread,          	    /* The update function      */
        create_fileread_tab,                /* The config tab create function   */
        apply_fileread_config,              /* Apply the config function        */
        save_fileread_config,               /* Save user config */
        load_fileread_config,               /* Load user config */
        "fileread",                         /* config keyword */
        NULL,                               /* Undefined 2  */
        NULL,                               /* Undefined 1  */
        NULL,                               /* Undefined 0  */
        MON_APM,                            /* Insert plugin before this monitor */
        NULL,                               /* Handle if a plugin, filled in by GKrellM     */
        NULL                                /* path if a plugin, filled in by GKrellM       */
        };

Monitor *
init_plugin()
        {
	fileread_file_paths=g_array_new(FALSE,TRUE,sizeof(item_t));
	fileread_file_path_num=0;
        return &plugin_mon;
        }
