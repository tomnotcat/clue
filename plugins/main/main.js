Gimo = imports.gi.Gimo;
Gtk = imports.gi.Gtk;
Ctk = imports.gi.Ctk;
GetText = imports.gettext;
_ = GetText.gettext;

clue_main_plugin = null;

Ctk.DecorateWindow = function (win) {
    var decorator = new Ctk.WindowDecorator ();
    decorator.attach (win);
}

function _clue_main_start (plugin)
{
    var builder = new Gtk.Builder ();
    builder.add_from_file (plugin.get_path () + "/main.ui", null);

    var win = builder.get_object ("main-window");
    Ctk.DecorateWindow (win);

    win.connect ("destroy", function (){
        Gtk.main_quit()
    });
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