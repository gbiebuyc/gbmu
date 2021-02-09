#include <gtk/gtk.h>
#include "../libretro.h"
#include <stdarg.h>
#include <stdio.h>

GtkWidget *window;
GtkWidget *image;
GdkPixbuf *pixbuf;
enum retro_pixel_format pixel_format = RETRO_PIXEL_FORMAT_RGB565;

// GTK Callbacks

bool key_press_event(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
	if (event->keyval == GDK_KEY_Escape) {
		gtk_main_quit();
		return TRUE;
	}
	return FALSE;
}

bool timeout_cb(void *data) {
	retro_run();
	return true;
}

// Libretro Callbacks

void log_cb(enum retro_log_level level, const char *fmt, ...) {
	(void)level;
	va_list args;
	va_start(args, fmt);
	vprintf(fmt, args);
	fflush(stdout);
	va_end(args);
}

bool environment_cb(unsigned cmd, void *data) {
	switch (cmd) {
		case RETRO_ENVIRONMENT_SET_PIXEL_FORMAT: {
			pixel_format = *(enum retro_pixel_format*)data;
			printf("RETRO_ENVIRONMENT_SET_PIXEL_FORMAT: %d\n", pixel_format);
			return TRUE;
		}
		case RETRO_ENVIRONMENT_GET_LOG_INTERFACE:
			((struct retro_log_callback*)data)->log = log_cb;
			return TRUE;
		case RETRO_ENVIRONMENT_GET_CAN_DUPE:
			*(bool*)data = TRUE;
			return TRUE;
		case RETRO_ENVIRONMENT_GET_VARIABLE_UPDATE:
			return FALSE;
		case RETRO_ENVIRONMENT_GET_VARIABLE: {
			struct retro_variable* variableData = (struct retro_variable *)data;
			printf("RETRO_ENVIRONMENT_GET_VARIABLE: %s\n", variableData->key);
			return FALSE;
		}
	}
	printf("environment command: %u\n", cmd);
	return FALSE;
}

void put_pixel(GdkPixbuf *pixbuf, int x, int y, guchar red, guchar green, guchar blue, guchar alpha)
{
	guchar *pixels, *p;
	int rowstride, numchannels;

	numchannels = gdk_pixbuf_get_n_channels(pixbuf);
	rowstride = gdk_pixbuf_get_rowstride(pixbuf);
	pixels = gdk_pixbuf_get_pixels(pixbuf);

	p = pixels + y * rowstride + x * numchannels;

	p[0] = red;
	p[1] = green;
	p[2] = blue;
	p[3] = alpha;
}

void video_refresh_cb(const void *data, unsigned width, unsigned height, size_t pitch) {
	printf(".");
	fflush(stdout);
	if (data == NULL)
		return;
	for (int y=0; y<144; y++) {
		for (int x=0; x<160; x++) {
			uint16_t px = ((uint16_t*)data)[y*pitch/sizeof(uint16_t) + x];
			uint32_t r = (px >> 11) * (256/32);
			uint32_t g = ((px >> 5) & 0x3f) * (256/64);
			uint32_t b = (px & 0x1f) * (256/32);
			put_pixel(pixbuf, x, y, r, g, b, 255);
		}
	}
	gtk_image_set_from_pixbuf((GtkImage*)image, pixbuf);
}

void audio_sample_cb(int16_t left, int16_t right) {
}

size_t audio_sample_batch_cb(const int16_t *data, size_t frames) {
}

void input_poll_cb(void) {
}

int16_t input_state_cb(unsigned port, unsigned device, unsigned index, unsigned id) {
}

int main(int ac, char **av) {
	struct retro_system_info systeminfo;
	struct retro_game_info gameinfo;

	// Init libretro core
	retro_get_system_info(&systeminfo);
	printf("[Gbmu] Core name: %s\n", systeminfo.library_name);
	retro_set_environment(environment_cb);
	retro_set_video_refresh(video_refresh_cb);
	retro_set_audio_sample(audio_sample_cb);
	retro_set_audio_sample_batch(audio_sample_batch_cb);
	retro_set_input_poll(input_poll_cb);
	retro_set_input_state(input_state_cb);
	retro_init();
	
	// Load ROM
	if (ac < 2)
		exit(puts("need rom path as argument"));
	gameinfo.path = av[1];
	gameinfo.data = malloc(8388608); // max 8 MB cartridges
	FILE *f = fopen(gameinfo.path, "rb");
	if (!f) exit(printf("fopen error: %s\n", gameinfo.path));
	gameinfo.size = 0;
	while (fread((void*)gameinfo.data + gameinfo.size, 1, 0x100, f) == 0x100)
		gameinfo.size += 0x100;
	if (!retro_load_game(&gameinfo))
		exit(puts("load game error"));

	// Init GTK
	gtk_init(&ac, &av);
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, 0, 8, 160, 144);
	image = gtk_image_new_from_pixbuf(pixbuf);
	gtk_container_add(GTK_CONTAINER(window), GTK_WIDGET(image));
	gtk_widget_show_all(window);
	g_signal_connect_swapped(G_OBJECT(window), "destroy", G_CALLBACK(gtk_main_quit), NULL);
	g_signal_connect(window, "key_press_event", G_CALLBACK(key_press_event), NULL);
	g_timeout_add(16, (GSourceFunc)timeout_cb, NULL);
	gtk_main();
}
