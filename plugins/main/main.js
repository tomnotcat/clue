const Gimo = imports.gi.Gimo;
const Gtk = imports.gi.Gtk;
const Ctk = imports.gi.Ctk;
const GtkBuilder = imports.gtkbuilder;
const GetText = imports.gettext;
const _ = GetText.gettext;

var clue = {};

function _clue_on_main_open (builder)
{
    var win = builder.get_object ("main-window");
    var filename = Ctk.open_single_file (win, null, null, null,
                                         ["Document Files (*.pdf;*.txt)",
                                          "*.pdf;*.txt;",
                                          "All Files (*.*)",
                                          "*.*"]);
    if (!filename)
        return;

    var doc = Ctk.load_document_from_file (clue.main.context, filename);
    if (!doc)
        return;

    if (!clue.main.docview) {
        var sw = builder.get_object ("doc-scrolledwindow");
        clue.main.docview = new Ctk.DocView ();
        sw.add (clue.main.docview);
        clue.main.docview.show ();
    }

    clue.main.docview.set_document (doc);
    var p = doc.get_page (0);
    print (p.get_size ());

    var split = filename.lastIndexOf ("\\");
    if (-1 == split)
        split = filename.lastIndexOf ("/");

    if (split != -1)
        win.set_title (filename.substring (split + 1));
    else
        win.set_title (filename);
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
    var win = builder.get_object ("main-window");
    win.destroy ();
}

function _clue_main_start (plugin)
{
    var builder = new Gtk.Builder ();

    builder.add_from_file (plugin.get_path () + "/main.ui", null);
    builder.connect_signals ({on_main_window_destroy:
                              function () { Gtk.main_quit ();},
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
    clue.main = {};
    clue.main.plugin = plugin;
    clue.main.context = plugin.query_context ();
    clue.main.window = win;
    clue.main.builder = builder;

    return true;
}

function clue_main_plugin (plugin)
{
    GetText.bindtextdomain ("clue", "main/locale");
    GetText.bindtextdomain ("gtk30", "main/locale");
    GetText.textdomain ("clue");
    plugin.connect ("start", _clue_main_start);
    return plugin;
}