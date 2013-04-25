const Gimo = imports.gi.Gimo;
const Gtk = imports.gi.Gtk;
const Ctk = imports.gi.Ctk;
const GtkBuilder = imports.gtkbuilder;
const GetText = imports.gettext;
const _ = GetText.gettext;

clue_main_plugin = null;

function _clue_on_main_open (builder)
{
    var parent = builder.get_object ("main-window");
    var filename = Ctk.open_single_file (parent, null, null, null,
                                         ["Document Files (*.pdf;*.txt)",
                                          "*.pdf;*.txt;",
                                          "All Files (*.*)",
                                          "*.*"]);
    if (!filename)
        return;

    var doc = Ctk.load_document (clue_main_plugin.context, filename);
    print (doc);
}

function _clue_on_main_save (builder)
{
    print ("hello save");
}

function _clue_on_main_save_as (builder)
{
    print ("hello save as");
}

function _clue_on_main_quit (builder)
{
    Gtk.main_quit ();
}

function _clue_main_start (plugin)
{
    var builder = new Gtk.Builder ();

    builder.add_from_file (plugin.get_path () + "/main.ui", null);
    builder.connect_signals ({on_main_window_destroy:
                              function () {_clue_on_main_quit (builder);},
                              on_action_open:
                              function () { _clue_on_main_open (builder);},
                              on_action_save:
                              function () { _clue_on_main_save (builder);},
                              on_action_save_as:
                              function () { _clue_on_main_save_as (builder);},
                              on_file_quit:
                              function () { _clue_on_main_quit (builder);}});

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