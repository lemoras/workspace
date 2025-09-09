#include <gtk/gtk.h>
#include <json-c/json.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define JSON_FILE "local-config.json"

// TreeStore columns
enum {
    COL_ID,
    COL_APP,
    COL_VERSION,
    COL_PUBLISH,
    COL_LANDING,
    COL_DOMAINS,
    COL_NAME_EN,
    COL_NAME_TR,
    COL_NAME_RU,
    COL_LABEL_EN,
    COL_LABEL_TR,
    COL_LABEL_RU,
    NUM_COLS
};

// Global pointers
GtkListStore *store;
GtkWidget *id_entry, *app_entry, *version_entry, *domains_entry;
GtkWidget *publish_check, *landing_check;
GtkWidget *name_en_entry, *name_tr_entry, *name_ru_entry;
GtkWidget *label_en_entry, *label_tr_entry, *label_ru_entry;
GtkTreeSelection *selection;

// Load JSON into TreeStore
void load_json() {
    FILE *fp = fopen(JSON_FILE, "r");
    if (!fp) return;
    fseek(fp, 0, SEEK_END);
    long len = ftell(fp);
    rewind(fp);

    char *data = malloc(len + 1);
    fread(data, 1, len, fp);
    data[len] = '\0';
    fclose(fp);

    struct json_object *root = json_tokener_parse(data);
    free(data);
    if (!root || !json_object_is_type(root, json_type_array)) return;

    int arr_len = json_object_array_length(root);
    for (int i = 0; i < arr_len; i++) {
        struct json_object *obj = json_object_array_get_idx(root, i);
        struct json_object *tmp;

        int _id = -1;
        const char *app = "", *version = "", *domains = "";
        gboolean publish = FALSE, landing = FALSE;
        const char *name_en="", *name_tr="", *name_ru="";
        const char *label_en="", *label_tr="", *label_ru="";

        if (json_object_object_get_ex(obj, "_id", &tmp)) _id = json_object_get_int(tmp);
        if (json_object_object_get_ex(obj, "app", &tmp)) app = json_object_get_string(tmp);
        if (json_object_object_get_ex(obj, "publish", &tmp)) publish = json_object_get_boolean(tmp);
        if (json_object_object_get_ex(obj, "landingPage", &tmp)) landing = json_object_get_boolean(tmp);
        if (json_object_object_get_ex(obj, "info", &tmp)) {
            struct json_object *ver;
            if (json_object_object_get_ex(tmp, "version", &ver)) version = json_object_get_string(ver);

            struct json_object *appName;
            if (json_object_object_get_ex(tmp, "appName", &appName)) {
                struct json_object *v;
                if (json_object_object_get_ex(appName, "en", &v)) name_en = json_object_get_string(v);
                if (json_object_object_get_ex(appName, "tr", &v)) name_tr = json_object_get_string(v);
                if (json_object_object_get_ex(appName, "ru", &v)) name_ru = json_object_get_string(v);
            }

            struct json_object *label;
            if (json_object_object_get_ex(tmp, "label", &label)) {
                struct json_object *v;
                if (json_object_object_get_ex(label, "en", &v)) label_en = json_object_get_string(v);
                if (json_object_object_get_ex(label, "tr", &v)) label_tr = json_object_get_string(v);
                if (json_object_object_get_ex(label, "ru", &v)) label_ru = json_object_get_string(v);
            }
        }

        if (json_object_object_get_ex(obj, "domains", &tmp) && json_object_is_type(tmp, json_type_array)) {
            char buf[512]="";
            for (int j=0;j<json_object_array_length(tmp);j++){
                const char *d=json_object_get_string(json_object_array_get_idx(tmp,j));
                strcat(buf,d);
                if(j!=json_object_array_length(tmp)-1) strcat(buf,", ");
            }
            domains = strdup(buf);
        }

        GtkTreeIter iter;
        gtk_list_store_append(store,&iter);
        gtk_list_store_set(store,&iter,
            COL_ID,_id,
            COL_APP,app,
            COL_VERSION,version,
            COL_PUBLISH,publish,
            COL_LANDING,landing,
            COL_DOMAINS,domains,
            COL_NAME_EN,name_en,
            COL_NAME_TR,name_tr,
            COL_NAME_RU,name_ru,
            COL_LABEL_EN,label_en,
            COL_LABEL_TR,label_tr,
            COL_LABEL_RU,label_ru,
            -1
        );
        if (domains) free((void*)domains);
    }
    json_object_put(root);
}

// Save TreeStore to JSON
void save_json() {
    GtkTreeIter iter;
    gboolean valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(store), &iter);
    struct json_object *root = json_object_new_array();

    while(valid){
        int _id;
        gboolean publish,landing;
        gchar *app,*version,*domains;
        gchar *name_en,*name_tr,*name_ru,*label_en,*label_tr,*label_ru;

        gtk_tree_model_get(GTK_TREE_MODEL(store),&iter,
            COL_ID,&_id,
            COL_APP,&app,
            COL_VERSION,&version,
            COL_PUBLISH,&publish,
            COL_LANDING,&landing,
            COL_DOMAINS,&domains,
            COL_NAME_EN,&name_en,
            COL_NAME_TR,&name_tr,
            COL_NAME_RU,&name_ru,
            COL_LABEL_EN,&label_en,
            COL_LABEL_TR,&label_tr,
            COL_LABEL_RU,&label_ru,
            -1
        );

        struct json_object *obj = json_object_new_object();
        json_object_object_add(obj,"_id",json_object_new_int(_id));
        json_object_object_add(obj,"app",json_object_new_string(app));
        json_object_object_add(obj,"publish",json_object_new_boolean(publish));
        json_object_object_add(obj,"landingPage",json_object_new_boolean(landing));

        struct json_object *info=json_object_new_object();
        json_object_object_add(info,"version",json_object_new_string(version));

        struct json_object *appName=json_object_new_object();
        json_object_object_add(appName,"en",json_object_new_string(name_en));
        json_object_object_add(appName,"tr",json_object_new_string(name_tr));
        json_object_object_add(appName,"ru",json_object_new_string(name_ru));
        json_object_object_add(info,"appName",appName);

        struct json_object *label=json_object_new_object();
        json_object_object_add(label,"en",json_object_new_string(label_en));
        json_object_object_add(label,"tr",json_object_new_string(label_tr));
        json_object_object_add(label,"ru",json_object_new_string(label_ru));
        json_object_object_add(info,"label",label);

        json_object_object_add(obj,"info",info);

        struct json_object *domains_array=json_object_new_array();
        char *token=strtok(domains,",");
        while(token){
            while(*token==' ') token++;
            json_object_array_add(domains_array,json_object_new_string(token));
            token=strtok(NULL,",");
        }
        json_object_object_add(obj,"domains",domains_array);

        json_object_array_add(root,obj);

        g_free(app); g_free(version); g_free(domains);
        g_free(name_en); g_free(name_tr); g_free(name_ru);
        g_free(label_en); g_free(label_tr); g_free(label_ru);

        valid=gtk_tree_model_iter_next(GTK_TREE_MODEL(store),&iter);
    }

    FILE *fp=fopen(JSON_FILE,"w");
    if(fp){
        fprintf(fp,"%s",json_object_to_json_string_ext(root,JSON_C_TO_STRING_PRETTY));
        fclose(fp);
    }
    json_object_put(root);
}

// TreeView checkbox toggle
void on_toggle_toggled(GtkCellRendererToggle *cell, gchar *path_str, gpointer user_data){
    GtkTreePath *path = gtk_tree_path_new_from_string(path_str);
    GtkTreeIter iter;
    gtk_tree_model_get_iter(GTK_TREE_MODEL(store), &iter, path);
    gboolean value;
    int col = GPOINTER_TO_INT(user_data);
    gtk_tree_model_get(GTK_TREE_MODEL(store), &iter, col, &value, -1);
    value = !value;
    gtk_list_store_set(store, &iter, col, value, -1);
    gtk_tree_path_free(path);
    save_json();
}

// Fill right panel
void on_row_selected(GtkTreeSelection *sel, gpointer user_data){
    GtkTreeModel *model;
    GtkTreeIter iter;
    if(gtk_tree_selection_get_selected(sel,&model,&iter)){
        int _id;
        gboolean publish,landing;
        gchar *app,*version,*domains;
        gchar *name_en,*name_tr,*name_ru,*label_en,*label_tr,*label_ru;

        gtk_tree_model_get(model,&iter,
            COL_ID,&_id,
            COL_APP,&app,
            COL_VERSION,&version,
            COL_PUBLISH,&publish,
            COL_LANDING,&landing,
            COL_DOMAINS,&domains,
            COL_NAME_EN,&name_en,
            COL_NAME_TR,&name_tr,
            COL_NAME_RU,&name_ru,
            COL_LABEL_EN,&label_en,
            COL_LABEL_TR,&label_tr,
            COL_LABEL_RU,&label_ru,
            -1
        );

        char buf[32];
        sprintf(buf,"%d",_id);
        gtk_entry_set_text(GTK_ENTRY(id_entry),buf);
        gtk_entry_set_text(GTK_ENTRY(app_entry),app);
        gtk_entry_set_text(GTK_ENTRY(version_entry),version);
        gtk_entry_set_text(GTK_ENTRY(domains_entry),domains);
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(publish_check),publish);
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(landing_check),landing);
        gtk_entry_set_text(GTK_ENTRY(name_en_entry),name_en);
        gtk_entry_set_text(GTK_ENTRY(name_tr_entry),name_tr);
        gtk_entry_set_text(GTK_ENTRY(name_ru_entry),name_ru);
        gtk_entry_set_text(GTK_ENTRY(label_en_entry),label_en);
        gtk_entry_set_text(GTK_ENTRY(label_tr_entry),label_tr);
        gtk_entry_set_text(GTK_ENTRY(label_ru_entry),label_ru);

        g_free(app); g_free(version); g_free(domains);
        g_free(name_en); g_free(name_tr); g_free(name_ru);
        g_free(label_en); g_free(label_tr); g_free(label_ru);
    }
}

// Apply right panel edits
void on_apply_clicked(GtkButton *btn,gpointer user_data){
    GtkTreeModel *model;
    GtkTreeIter iter;
    if(gtk_tree_selection_get_selected(selection,&model,&iter)){
        int _id=atoi(gtk_entry_get_text(GTK_ENTRY(id_entry)));
        const char *app=gtk_entry_get_text(GTK_ENTRY(app_entry));
        const char *version=gtk_entry_get_text(GTK_ENTRY(version_entry));
        const char *domains=gtk_entry_get_text(GTK_ENTRY(domains_entry));
        gboolean publish=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(publish_check));
        gboolean landing=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(landing_check));
        const char *name_en=gtk_entry_get_text(GTK_ENTRY(name_en_entry));
        const char *name_tr=gtk_entry_get_text(GTK_ENTRY(name_tr_entry));
        const char *name_ru=gtk_entry_get_text(GTK_ENTRY(name_ru_entry));
        const char *label_en=gtk_entry_get_text(GTK_ENTRY(label_en_entry));
        const char *label_tr=gtk_entry_get_text(GTK_ENTRY(label_tr_entry));
        const char *label_ru=gtk_entry_get_text(GTK_ENTRY(label_ru_entry));

        gtk_list_store_set(store,&iter,
            COL_ID,_id,
            COL_APP,app,
            COL_VERSION,version,
            COL_PUBLISH,publish,
            COL_LANDING,landing,
            COL_DOMAINS,domains,
            COL_NAME_EN,name_en,
            COL_NAME_TR,name_tr,
            COL_NAME_RU,name_ru,
            COL_LABEL_EN,label_en,
            COL_LABEL_TR,label_tr,
            COL_LABEL_RU,label_ru,
            -1
        );
        save_json();
    }
}

// Add / Remove
void on_add_clicked(GtkButton *btn,gpointer user_data){
    GtkTreeIter iter;
    gtk_list_store_append(store,&iter);
    gtk_list_store_set(store,&iter,
        COL_ID,-1,
        COL_APP,"",
        COL_VERSION,"1.0.0",
        COL_PUBLISH,FALSE,
        COL_LANDING,FALSE,
        COL_DOMAINS,"",
        COL_NAME_EN,"",
        COL_NAME_TR,"",
        COL_NAME_RU,"",
        COL_LABEL_EN,"",
        COL_LABEL_TR,"",
        COL_LABEL_RU,"",
        -1
    );
    save_json();
}
void on_remove_clicked(GtkButton *btn,gpointer user_data){
    GtkTreeModel *model;
    GtkTreeIter iter;
    if(gtk_tree_selection_get_selected(selection,&model,&iter)){
        gtk_list_store_remove(store,&iter);
        save_json();
    }
}

int main(int argc,char *argv[]){
    gtk_init(&argc,&argv);

    GtkWidget *window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window),"Config Manager");
    gtk_window_set_default_size(GTK_WINDOW(window),1200,600);
    g_signal_connect(window,"destroy",G_CALLBACK(gtk_main_quit),NULL);

    // TreeStore
    store=gtk_list_store_new(NUM_COLS,
        G_TYPE_INT,     // _id
        G_TYPE_STRING,  // app
        G_TYPE_STRING,  // version
        G_TYPE_BOOLEAN, // publish
        G_TYPE_BOOLEAN, // landing
        G_TYPE_STRING,  // domains
        G_TYPE_STRING,  // name_en
        G_TYPE_STRING,  // name_tr
        G_TYPE_STRING,  // name_ru
        G_TYPE_STRING,  // label_en
        G_TYPE_STRING,  // label_tr
        G_TYPE_STRING   // label_ru
    );
    load_json();

    // TreeView
    GtkWidget *view=gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *col;

    renderer=gtk_cell_renderer_text_new();
    col=gtk_tree_view_column_new_with_attributes("_id",renderer,"text",COL_ID,NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(view),col);

    renderer=gtk_cell_renderer_text_new();
    col=gtk_tree_view_column_new_with_attributes("App",renderer,"text",COL_APP,NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(view),col);

    renderer=gtk_cell_renderer_text_new();
    col=gtk_tree_view_column_new_with_attributes("Version",renderer,"text",COL_VERSION,NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(view),col);

    renderer=gtk_cell_renderer_toggle_new();
    g_signal_connect(renderer,"toggled",G_CALLBACK(on_toggle_toggled),GINT_TO_POINTER(COL_PUBLISH));
    col=gtk_tree_view_column_new_with_attributes("Publish",renderer,"active",COL_PUBLISH,NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(view),col);

    renderer=gtk_cell_renderer_toggle_new();
    g_signal_connect(renderer,"toggled",G_CALLBACK(on_toggle_toggled),GINT_TO_POINTER(COL_LANDING));
    col=gtk_tree_view_column_new_with_attributes("Landing",renderer,"active",COL_LANDING,NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(view),col);

    renderer=gtk_cell_renderer_text_new();
    col=gtk_tree_view_column_new_with_attributes("Domains",renderer,"text",COL_DOMAINS,NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(view),col);

    selection=gtk_tree_view_get_selection(GTK_TREE_VIEW(view));
    gtk_tree_selection_set_mode(selection,GTK_SELECTION_SINGLE);
    g_signal_connect(selection,"changed",G_CALLBACK(on_row_selected),NULL);

    // Right panel notebook
    GtkWidget *notebook=gtk_notebook_new();

    // General tab
    GtkWidget *grid=gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid),5);
    gtk_grid_set_column_spacing(GTK_GRID(grid),5);
    id_entry=gtk_entry_new();
    app_entry=gtk_entry_new();
    version_entry=gtk_entry_new();
    domains_entry=gtk_entry_new();
    publish_check=gtk_check_button_new_with_label("Publish");
    landing_check=gtk_check_button_new_with_label("Landing Page");
    GtkWidget *apply_btn=gtk_button_new_with_label("Apply");
    g_signal_connect(apply_btn,"clicked",G_CALLBACK(on_apply_clicked),NULL);

    gtk_grid_attach(GTK_GRID(grid),gtk_label_new("_id:"),0,0,1,1);
    gtk_grid_attach(GTK_GRID(grid),id_entry,1,0,1,1);
    gtk_grid_attach(GTK_GRID(grid),gtk_label_new("App:"),0,1,1,1);
    gtk_grid_attach(GTK_GRID(grid),app_entry,1,1,1,1);
    gtk_grid_attach(GTK_GRID(grid),gtk_label_new("Version:"),0,2,1,1);
    gtk_grid_attach(GTK_GRID(grid),version_entry,1,2,1,1);
    gtk_grid_attach(GTK_GRID(grid),GTK_WIDGET(publish_check),0,3,2,1);
    gtk_grid_attach(GTK_GRID(grid),GTK_WIDGET(landing_check),0,4,2,1);
    gtk_grid_attach(GTK_GRID(grid),gtk_label_new("Domains:"),0,5,1,1);
    gtk_grid_attach(GTK_GRID(grid),domains_entry,1,5,1,1);
    gtk_grid_attach(GTK_GRID(grid),apply_btn,0,6,2,1);

    gtk_notebook_append_page(GTK_NOTEBOOK(notebook),grid,gtk_label_new("General"));

    // Names tab
    GtkWidget *names_grid=gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(names_grid),5);
    gtk_grid_set_column_spacing(GTK_GRID(names_grid),5);
    name_en_entry=gtk_entry_new();
    name_tr_entry=gtk_entry_new();
    name_ru_entry=gtk_entry_new();
    gtk_grid_attach(GTK_GRID(names_grid),gtk_label_new("Name EN:"),0,0,1,1);
    gtk_grid_attach(GTK_GRID(names_grid),name_en_entry,1,0,1,1);
    gtk_grid_attach(GTK_GRID(names_grid),gtk_label_new("Name TR:"),0,1,1,1);
    gtk_grid_attach(GTK_GRID(names_grid),name_tr_entry,1,1,1,1);
    gtk_grid_attach(GTK_GRID(names_grid),gtk_label_new("Name RU:"),0,2,1,1);
    gtk_grid_attach(GTK_GRID(names_grid),name_ru_entry,1,2,1,1);
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook),names_grid,gtk_label_new("Names"));

    // Labels tab
    GtkWidget *labels_grid=gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(labels_grid),5);
    gtk_grid_set_column_spacing(GTK_GRID(labels_grid),5);
    label_en_entry=gtk_entry_new();
    label_tr_entry=gtk_entry_new();
    label_ru_entry=gtk_entry_new();
    gtk_grid_attach(GTK_GRID(labels_grid),gtk_label_new("Label EN:"),0,0,1,1);
    gtk_grid_attach(GTK_GRID(labels_grid),label_en_entry,1,0,1,1);
    gtk_grid_attach(GTK_GRID(labels_grid),gtk_label_new("Label TR:"),0,1,1,1);
    gtk_grid_attach(GTK_GRID(labels_grid),label_tr_entry,1,1,1,1);
    gtk_grid_attach(GTK_GRID(labels_grid),gtk_label_new("Label RU:"),0,2,1,1);
    gtk_grid_attach(GTK_GRID(labels_grid),label_ru_entry,1,2,1,1);
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook),labels_grid,gtk_label_new("Labels"));

    // Buttons
    GtkWidget *add_btn=gtk_button_new_with_label("Add");
    GtkWidget *remove_btn=gtk_button_new_with_label("Remove");
    g_signal_connect(add_btn,"clicked",G_CALLBACK(on_add_clicked),NULL);
    g_signal_connect(remove_btn,"clicked",G_CALLBACK(on_remove_clicked),NULL);
    GtkWidget *button_box=gtk_box_new(GTK_ORIENTATION_HORIZONTAL,5);
    gtk_box_pack_start(GTK_BOX(button_box),add_btn,TRUE,TRUE,0);
    gtk_box_pack_start(GTK_BOX(button_box),remove_btn,TRUE,TRUE,0);

    // Main layout
    GtkWidget *hbox=gtk_box_new(GTK_ORIENTATION_HORIZONTAL,10);
    gtk_box_pack_start(GTK_BOX(hbox),view,TRUE,TRUE,0);
    gtk_box_pack_start(GTK_BOX(hbox),notebook,FALSE,FALSE,0);

    GtkWidget *vbox=gtk_box_new(GTK_ORIENTATION_VERTICAL,5);
    gtk_box_pack_start(GTK_BOX(vbox),hbox,TRUE,TRUE,0);
    gtk_box_pack_start(GTK_BOX(vbox),button_box,FALSE,FALSE,0);

    gtk_container_add(GTK_CONTAINER(window),vbox);
    gtk_widget_show_all(window);
    gtk_main();
    return 0;
}
