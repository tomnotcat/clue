const Gimo = imports.gi.Gimo;
const Gtk = imports.gi.Gtk;
const Ctk = imports.gi.Ctk;
const GtkBuilder = imports.gtkbuilder;
const GetText = imports.gettext;
const _ = GetText.gettext;

clue_main_plugin = null;

function _clue_main_window_destroy ()
{
    Gtk.main_quit ();
}

function _clue_main_start (plugin)
{
    var builder = new Gtk.Builder ();
    builder.add_from_file (plugin.get_path () + "/main.ui", null);
    builder.connect_signals ({on_main_window_destroy: _clue_main_window_destroy});

    var win = builder.get_object ("main-window");
    win.decorate ();
    win.show ();

    plugin.define_object ("main-window", win);

    // setup global object
    clue_main_plugin = plugin;
    clue_main_plugin.context = plugin.query_context ();
    clue_main_plugin.window = win;
    clue_main_plugin.builder = builder;

    return true;
}

function clue_main_plugin_init (plugin)
{
    GetText.bindtextdomain ("clue", "main/locale");
    GetText.bindtextdomain ("gtk30", "main/locale");
    GetText.textdomain ("clue");
    plugin.connect ("start", _clue_main_start);
    return plugin;
}