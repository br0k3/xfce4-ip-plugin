#include <gtk/gtk.h>
#include <libxfce4panel/libxfce4panel.h>
#include <libxfce4ui/libxfce4ui.h>
#include <libxfce4util/libxfce4util.h>
#include <libnotify/notify.h>

// About
#define APPNAME "Public IP Plugin"
#define VERSION "0.1.1"
#define COPYRIGHT "Copyright \302\251 2026 br0k3"
#define COMMENTS "Adds an icon to show your public IP on mouse hover, with a copy to clipboard action."
#define WEBSITE "https://github.com/br0k3/xfce4-ip-plugin"

typedef struct {
    XfcePanelPlugin *plugin;
    GtkWidget       *button;
    GtkWidget       *icon;
    gchar           *current_ip;
    gboolean         use_async;
} IPPlugin;

/* Helper struct to pass data back to the UI thread */
typedef struct {
    IPPlugin *ip_plug;
    gchar    *ip_string;
} ResultData;

/* --- REFRESH LOGIC --- */
static gchar *perform_curl() {
    gchar *stdout_buf = NULL;
    if (g_spawn_command_line_sync("curl -4 -s --max-time 5 https://api.ipify.org", &stdout_buf, NULL, NULL, NULL)) {
        if (stdout_buf && strlen(stdout_buf) > 0) return g_strstrip(stdout_buf);
    }
    return g_strdup("Offline");
}

static void update_ui(IPPlugin *ip_plug, const gchar *ip) {
    g_free(ip_plug->current_ip);
    ip_plug->current_ip = g_strdup(ip);
    gchar *tooltip = g_strdup_printf("Public IP: %s\nClick to copy", ip);
    gtk_widget_set_tooltip_text(ip_plug->button, tooltip);
    g_free(tooltip);
}

static gboolean update_ui_idle(ResultData *rd) {
    update_ui(rd->ip_plug, rd->ip_string);
    g_free(rd->ip_string);
    g_free(rd);
    return FALSE;
}

static gpointer fetch_ip_async_thread(gpointer data) {
    IPPlugin *ip_plug = (IPPlugin *)data;
    ResultData *rd = g_new0(ResultData, 1);
    rd->ip_plug = ip_plug;
    rd->ip_string = perform_curl();
    
    g_idle_add((GSourceFunc)update_ui_idle, rd);
    return NULL;
}

static void trigger_update(gpointer data) {
    IPPlugin *ip_plug = (IPPlugin *)data;
    if (ip_plug->use_async) {
        g_thread_new("ip-fetch", fetch_ip_async_thread, ip_plug);
    } else {
        gchar *ip = perform_curl();
        update_ui(ip_plug, ip);
        g_free(ip);
    }
    return TRUE;
}

static void menu_refresh_clicked(GtkMenuItem *item, IPPlugin *ip_plug) {
    // Re-use our existing trigger function
    trigger_update(ip_plug);
}

static void send_notification(const gchar *ip) {
    if (!notify_is_initted()) notify_init("IP Plugin");
    NotifyNotification *n = notify_notification_new("IP Copied", ip, "applications-internet");
    notify_notification_set_timeout(n, 3000);
    notify_notification_show(n, NULL);
    g_object_unref(G_OBJECT(n));
}

static gboolean ip_clicked(GtkWidget *widget, GdkEventButton *event, IPPlugin *ip_plug) {
    if (event->button == 1) { // Left click
        if (ip_plug->current_ip && g_strcmp0(ip_plug->current_ip, "...") != 0) {
            GtkClipboard *clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
            gtk_clipboard_set_text(clipboard, ip_plug->current_ip, -1);
            send_notification(ip_plug->current_ip);
        }
    }
    return FALSE;
}

void show_about()
{
	const gchar* authors[] = {
		"br0k3 <generated110@hotmail.com>",
		NULL };

	gtk_show_about_dialog
		(NULL,
		"authors", authors,
		"comments", _(COMMENTS),
		"copyright", COPYRIGHT,
		"license", "GPL 2.0",
		"logo-icon-name", "applications-internet",
		"program-name", APPNAME,
		"version", VERSION,
		"website", WEBSITE,
		NULL);
}

/* --- CONSTRUCTOR --- */
static void ip_plugin_construct(XfcePanelPlugin *plugin) {
    IPPlugin *ip_plug = g_new0(IPPlugin, 1);
    ip_plug->plugin = plugin;
    ip_plug->current_ip = g_strdup("...");
    ip_plug->use_async = TRUE;

    notify_init("Xfce IP Plugin");

    /* UI Setup */
    ip_plug->button = xfce_panel_create_button();
    ip_plug->icon = gtk_image_new_from_icon_name("applications-internet", GTK_ICON_SIZE_BUTTON);
    gtk_container_add(GTK_CONTAINER(ip_plug->button), ip_plug->icon);
    gtk_container_add(GTK_CONTAINER(plugin), ip_plug->button);
    gtk_widget_show_all(ip_plug->button);

	GtkWidget *item, *image, *abt, *abtImg;

    /* Create the "Refresh IP" menu item */
    item = gtk_image_menu_item_new_with_label("Refresh IP Now");
    image = gtk_image_new_from_icon_name("view-refresh", GTK_ICON_SIZE_MENU);
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(item), image);
    
    /* Create the "About" menu item */
    abt = gtk_image_menu_item_new_with_label("About");
    abtImg = gtk_image_new_from_icon_name("help-about", GTK_ICON_SIZE_MENU);
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(abt), abtImg);
    
    /* Connect the click events */
    g_signal_connect(G_OBJECT(item), "activate", G_CALLBACK(menu_refresh_clicked), ip_plug);
    g_signal_connect(G_OBJECT(abt), "activate", G_CALLBACK(show_about), NULL);
    
    gtk_widget_show(item);
    gtk_widget_show(abt);
    
    /* Insert it in the the menu */
    xfce_panel_plugin_menu_insert_item(plugin, GTK_MENU_ITEM(item));
    xfce_panel_plugin_menu_insert_item(plugin, GTK_MENU_ITEM(abt));
	
    g_signal_connect(ip_plug->button, "button-press-event", G_CALLBACK(ip_clicked), ip_plug);
    xfce_panel_plugin_add_action_widget(plugin, ip_plug->button);
    
    trigger_update(ip_plug);
    g_timeout_add_seconds(1800, trigger_update, ip_plug);
}

XFCE_PANEL_PLUGIN_REGISTER(ip_plugin_construct);
