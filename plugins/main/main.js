const Gimo = imports.gi.Gimo;
const Oren = imports.gi.Oren;
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

    if (!clue.main.thread_pool)
        clue.main.thread_pool = new Oren.ThreadPool ({thread_count: 2});

    if (!clue.main.docmodel) {
        clue.main.docmodel = new Ctk.DocModel ();
        clue.main.docmodel.set_min_scale (0.1);
        clue.main.docmodel.set_max_scale (4.0);
    }

    if (!clue.main.docview) {
        var sw = builder.get_object ("doc-scrolledwindow");
        clue.main.docview = new Ctk.DocView ({thread_pool: clue.main.thread_pool});
        clue.main.docview.set_model (clue.main.docmodel);
        clue.main.docview.set_render_cache_size (1024 * 1024 * 30);
        sw.add (clue.main.docview);
        clue.main.docview.show ();
    }

    clue.main.docmodel.set_document (doc);

    var split = filename.lastIndexOf ("\\");
    if (-1 == split)
        split = filename.lastIndexOf ("/");

    if (split != -1)
        win.set_title (filename.substring (split + 1));
    else
        win.set_title (filename);
}

function _clue_on_main_rotate (builder, angle)
{
    var rotation = clue.main.docmodel.get_rotation ();
    clue.main.docmodel.set_rotation (rotation + angle);
}

function _clue_on_main_quit (builder)
{
    var win = builder.get_object ("main-window");
    win.destroy ();
}

function _clue_on_main_destroy (builder)
{
    builder.disconnect_signals ();
    Gtk.main_quit ();
}

function _clue_main_start (plugin)
{
    var builder = new Gtk.Builder ();

    builder.add_from_file (plugin.get_path () + "/main.ui", null);
    builder.connect_signals ({on_open_file:
                              function () { _clue_on_main_open (builder);},
                              on_rotate_left:
                              function () { _clue_on_main_rotate (builder, -90);},
                              on_rotate_right:
                              function () { _clue_on_main_rotate (builder, 90);},
                              on_file_quit:
                              function () { _clue_on_main_quit (builder);},
                              on_main_window_destroy:
                              function () { _clue_on_main_destroy (builder);}});

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