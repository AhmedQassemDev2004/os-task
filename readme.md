Install GTK

```sudo apt-get install libgtk-3-dev```

Run app

```$gcc pkg-config --cflags gtk+-3.0 -o os_task os_task.c pkg-config --libs gtk+-3.0```
