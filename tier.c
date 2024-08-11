#include <GLFW/glfw3.h>
#include <GL/gl.h>
#include <leif/leif.h>
#include <string.h>

#define WIN_GAP 20.0f
#define TIER_GAP 50.0f
#define TIER_ICON_WIDTH 70.0f
#define TIER_ICON_HEIGHT 70.0f
#define DIVIDER_WIDTH 1080.0f
#define DIVIDER_THICKNESS 1.0f
#define ITEM_CONTAINER_WIDTH 960.0f
#define ITEM_CONTAINER_HEIGHT 90.0f
#define ITEM_ICON_HEIGHT 10.0f

typedef enum {
    S_TIER = 0,
    A_TIER,
    B_TIER,
    C_TIER,
    D_TIER
} rating;

typedef enum {
    INIT = 0,
    ADD
} page;

typedef struct {
    char* name;
    char* tier;
    bool delete;
} item;

static int winw = 1080, winh = 610;
static LfFont titleFont, buttonFont, iconFont, itemFont, subtitleFont;
static LfColor* icon_colors;
static char** icon_letters;
static char ** ratings;
static float altitudes[5];
static page current_page;
static LfInputField new_item_input;
static char new_item_input_buf[512];


static item* item_entries[1024];
static uint32_t num_entries = 0;

static void rendertopbar() {

    // Top Bar Position
    lf_set_ptr_x_absolute(30.0f); lf_set_ptr_y_absolute(40.0f);

    // Title Config
    lf_push_font(&titleFont);
    LfUIElementProps prop = lf_get_theme().text_props;
    prop.margin_bottom = 0.0f; prop.margin_left = 0.0f; prop.margin_right = 0.0f; prop.margin_top = 0.0f;
    lf_push_style_props(prop);
    lf_text("S-D Tier Ranking Interface");
    lf_pop_font();
    lf_pop_style_props();

    // "NEW" Button Config
    const float newWidth = 90.0f;
    lf_set_ptr_x_absolute(winw - (1.5 * newWidth) - WIN_GAP); lf_set_ptr_y_absolute(25.0f);
    LfUIElementProps newprop = lf_get_theme().button_props;
    newprop.margin_left = 0.0f;
    newprop.margin_right = 0.0f;
    newprop.color = (LfColor){60, 179, 113, 244};
    newprop.border_width = 0.0f;
    newprop.corner_radius = 2.0f;
    lf_push_style_props(newprop);
    lf_set_line_should_overflow(false);
    lf_push_font(&buttonFont);

    if (lf_button_fixed("Add Item", newWidth, -1) == LF_CLICKED) {
        current_page = ADD;
    }

    lf_set_line_should_overflow(true);
    lf_pop_style_props();
    lf_pop_font;

}

static void rendericons(){
    float xpos = 25.0f, ypos = 100.0f;
    for (int i = 0; i < 5; i++) {
        LfUIElementProps prop = lf_get_theme().div_props;
        prop.corner_radius = 2.0f;
        prop.color = icon_colors[i];
        prop.border_width = 0.0f;
        lf_push_style_props(prop);

        // Container Position
        lf_div_begin(((vec2s){xpos, ypos}), ((vec2s) {TIER_ICON_WIDTH, TIER_ICON_HEIGHT}), false);
        lf_pop_style_props();

        // Icon Letter Placement
        lf_set_ptr_x_absolute(xpos + 20); lf_set_ptr_y_absolute(ypos + 20);

        // Setting up Font
        lf_push_font(&iconFont);

        // Letter Config
        LfUIElementProps text_prop = lf_get_theme().text_props;
        text_prop.margin_left = 0.0f; text_prop.margin_right = 0.0f;
        text_prop.margin_top = 0.0f; text_prop.margin_bottom = 0.0f;
        prop.text_color = (LfColor) LF_BLACK;
        lf_push_style_props(text_prop);
        lf_text(icon_letters[i]);

        // lf_pop_style_props();
        lf_pop_font();
        lf_pop_style_props();
        lf_div_end();

        ypos += 100.0f;
    }
}

static void renderdivisions() {
    float xpos = 0.0f, ypos = 85.0f;
    for (int i = 0; i < 6; i++) {
        LfUIElementProps prop = lf_get_theme().div_props;
        prop.color = (LfColor)LF_WHITE;
        prop.border_width = 0.0f;
        lf_push_style_props(prop);

        // Divider Position
        lf_div_begin(((vec2s){xpos, ypos}), ((vec2s) {DIVIDER_WIDTH, DIVIDER_THICKNESS}), false);
        lf_pop_style_props();
        lf_div_end();

        ypos += 100.0f;

    }
}

static void renderitemcontainers() {
    float xpos = 110.0f, ypos = 90.0f;
    for (int i = 0; i < 5; i ++) {
        LfUIElementProps prop = lf_get_theme().div_props;
        prop.margin_bottom = 0.0f; prop.margin_bottom = 0.0f; prop.margin_right = 0.0f; prop.margin_top = 0.0f;
        prop.color = (LfColor) {0.0f, 0.0f, 0.0f, 100.0f}; prop.border_width = 0.0f; prop.corner_radius = 1.0f;

        lf_push_style_props(prop);
        lf_div_begin(((vec2s){xpos, ypos}), ((vec2s){ITEM_CONTAINER_WIDTH, ITEM_CONTAINER_HEIGHT}), false);
        lf_pop_style_props();
        lf_set_line_should_overflow(true);
        lf_div_end();

        ypos += 100.0f;

    }
}

int total_length() {
    int total = 0;
    for (size_t i = 0; i < num_entries; i++) {
        total += strlen(item_entries[i]->name);
    }
    return total;
}

void renderitemplacement() {
    float* x_vals = malloc(5 * sizeof(float));
    x_vals[0] = 100.0f; x_vals[1] = 100.0f; x_vals[2] = 100.0f; x_vals[3] = 100.0f; x_vals[4] = 100.0f;

    for (uint32_t i = 0; i < num_entries; i++) {
        uint32_t result = 0;

        for (uint32_t j = 0; j < 5; j++) {
            if (strcmp(item_entries[i]->tier, ratings[j]) == 0) {
                result = j; // item's tier has been identified, result represents tier as #
            }
        }
        // Item Icon Config
        LfUIElementProps prop = lf_get_theme().button_props;
        prop.color = (LfColor) icon_colors[result]; prop.border_width = 0.0f;
        prop.corner_radius = 1.0f; prop.margin_left = 2.0f; prop.margin_right = 2.0f;

        lf_push_font(&itemFont);
        lf_push_style_props(prop);
        lf_set_ptr_x(x_vals[result]);
        x_vals[result] += 50 + (2 * strlen(item_entries[i]->name));
        lf_set_ptr_y_absolute(altitudes[result]);

        lf_button_fixed(item_entries[i]->name, 7 * strlen(item_entries[i]->name), ITEM_ICON_HEIGHT);

        lf_set_line_should_overflow(true);

        lf_pop_style_props();
        lf_pop_font();
    }

    free(x_vals);
}

static void serialize_entry(FILE* file, item* entry) {
    // Write name to file
    size_t namelen = strlen(entry->name) + 1;
    fwrite(&namelen, sizeof(size_t), 1, file);
    fwrite(entry->name, sizeof(char), namelen, file);

    // Write tier to file
    size_t tierlen = strlen(entry->tier) + 1;
    fwrite(&tierlen, sizeof(size_t), 1, file);
    fwrite(entry->tier, sizeof(char), tierlen, file);
}

static void serialize_list(const char* filename) {
    FILE* file = fopen(filename, "wb");
    if (!file) {
        printf("Failed to open file.\n");
        return;
    }

    for (uint32_t i = 0; i < num_entries; i++) {
        serialize_entry(file, item_entries[i]);
    }

    fclose(file);
}

item* deserialize_entry(FILE* file) {
    item* entry = malloc(sizeof(item));
    if (!entry) {
        return NULL;
    }

    size_t namelen, tierlen;
    if (fread(&namelen, sizeof(size_t), 1, file) != 1) {
        free(entry);
        return NULL;
    }

    entry->name = malloc(namelen);
    if (fread(entry->name, sizeof(char), namelen, file) != namelen) {
        free(entry->name);
        free(entry);
        return NULL;
    }

    if (fread(&tierlen, sizeof(size_t), 1, file) != 1) {
        free(entry->name);
        free(entry);
        return NULL;
    }

    entry->tier = malloc(tierlen);
    if (fread(entry->tier, sizeof(char), tierlen, file) != tierlen) {
        free(entry->name);
        free(entry->tier);
        free(entry);
        return NULL;
    }

    return entry;
}

void deserialize_list(const char* filename) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        printf("File not found. Creating new file.\n");
        file = fopen(filename, "wb");
        if (!file) {
            printf("Failed to create file.\n");
            return;
        }
        fclose(file);
        return;
    }

    item* entry;
    while((entry = deserialize_entry(file)) != NULL) {
        item_entries[num_entries++] = entry;
    }
    fclose(file);
}

void configs() {
    // Fonts
    titleFont = lf_load_font("./fonts/inter-bold.ttf", 30);
    buttonFont = lf_load_font("./fonts/inter-bold.ttf", 18);
    iconFont = lf_load_font("./fonts/inter-bold.ttf", 50);
    itemFont = lf_load_font("./fonts/inter-bold.ttf", 13);
    subtitleFont = lf_load_font("./fonts/inter.ttf", 20);
    
    // Icon colors/letters
    icon_colors = malloc(5 * sizeof(LfColor));
    icon_colors[0] = (LfColor){230, 65, 65, 244}; icon_colors[1] = (LfColor){230, 145, 65, 244}; icon_colors[2] = (LfColor){230, 230, 65, 244};
    icon_colors[3] = (LfColor){155, 230, 65, 244}; icon_colors[4] = (LfColor){65, 230, 65, 244};

    icon_letters = malloc(5 * sizeof(char*));
    icon_letters[0] = malloc(2 * sizeof(char)); icon_letters[0] = strdup("S");
    icon_letters[1] = malloc(2 * sizeof(char)); icon_letters[1] = strdup("A");
    icon_letters[2] = malloc(2 * sizeof(char)); icon_letters[2] = strdup("B");
    icon_letters[3] = malloc(2 * sizeof(char)); icon_letters[3] = strdup("C");
    icon_letters[4] = malloc(2 * sizeof(char)); icon_letters[4] = strdup("D");

    ratings = malloc(5 * sizeof(char*)); ratings[0] = strdup("S"); ratings[1] = strdup("A"); ratings[2] = strdup("B"); 
    ratings[3] = strdup("C"); ratings[4] = strdup("D");

    altitudes[0] = 115.0f; altitudes[1] = 215.0f; altitudes[2] = 315.0f; altitudes[3] = 415.0f; altitudes[4] = 515.0f;

    memset(new_item_input_buf, 0, sizeof(new_item_input));
    new_item_input = (LfInputField) {
        .width = 300,
        .buf = new_item_input_buf,
        .buf_size = sizeof(new_item_input_buf),
        .placeholder = (char*)"What item are you adding?"
    };

    //Upon Startup
    current_page = INIT;
}

static void rendernewpage() {

    static int32_t selected_tier = -1;

    static bool opened = false;

    lf_push_font(&titleFont);

    LfUIElementProps prop = lf_get_theme().text_props;
    prop.margin_bottom  = 15.0f;
    lf_push_style_props(prop);
    prop.text_color = LF_WHITE;
    lf_set_ptr_x_absolute(20.0f); lf_set_ptr_y_absolute(20.0f);
    lf_text("Create a new entry");
    lf_pop_font();

    lf_next_line();

    lf_push_font(&subtitleFont);
    lf_set_ptr_x_absolute(20.0f); lf_set_ptr_y_absolute(105.0f);
    lf_text("Item Name (Max 10 Characters)");
    lf_pop_font();

    lf_next_line();

    LfUIElementProps props = lf_get_theme().inputfield_props;
    props.padding = 15.0f; props.border_width = 2.0f;
    props.color = (LfColor) {0.05f, 0.05f, 0.05f, 1.0f};
    props.corner_radius = 3.0f;
    props.text_color = LF_WHITE;
    props.border_color = new_item_input.selected ? LF_WHITE : (LfColor) {0.1f, 0.1f, 0.1f, 1.0f};
    lf_set_ptr_x_absolute(15.0f); lf_set_ptr_y_absolute(130.0f);
    lf_push_style_props(props);
    lf_input_text(&new_item_input);
    lf_pop_style_props();

    lf_next_line();

    lf_push_font(&subtitleFont);
    lf_set_ptr_x_absolute(20.0f); lf_set_ptr_y_absolute(210.0f);
    lf_text("Tier:");
    lf_pop_font();

    lf_next_line();

    lf_set_line_should_overflow(true);

    LfUIElementProps dropprop = lf_get_theme().button_props;
    dropprop.color = (LfColor) {70, 70, 70, 255};
    dropprop.text_color = LF_WHITE;
    dropprop.border_width = 0.0f; dropprop.corner_radius = 3.0f;
    lf_set_ptr_x_absolute(60.0f); lf_set_ptr_y_absolute(200.0f);
    lf_push_style_props(dropprop);
    lf_dropdown_menu((const char**)ratings, "Select", 5, 100 , 120, &selected_tier, &opened);
    lf_pop_style_props();

    lf_next_line();

    bool form_complete = (strlen(new_item_input_buf) && (selected_tier != -1));
    const char* text = "Confirm";
    const float width = 75.0f;

    LfUIElementProps returnbutton = lf_get_theme().button_props;
    returnbutton.margin_left = 0.0f; returnbutton.margin_right = 0.0f;
    returnbutton.corner_radius = 2.0f; returnbutton.border_width = 0.0f;
    returnbutton.color = LF_BLUE;
    returnbutton.text_color = LF_WHITE;
    lf_push_style_props(returnbutton);

    lf_set_ptr_x_absolute(25.0f);
    lf_set_ptr_y_absolute(winh - (lf_button_dimension(text).y + returnbutton.padding * 2.0f) - WIN_GAP);

    if(lf_button_fixed("Back", width, 10.0f) == LF_CLICKED) {
        current_page = INIT;
        selected_tier = -1;
        opened = false;
    }

    lf_pop_style_props();
    lf_pop_style_props();

    LfUIElementProps button = lf_get_theme().button_props;
    button.margin_left = 0.0f; button.margin_right = 0.0f;
    button.corner_radius = 5.0f; button.border_width = 0.0f;
    button.color = !form_complete ? LF_RED : LF_GREEN;
    button.text_color = LF_WHITE;
    lf_push_style_props(button);
    lf_set_line_should_overflow(false);


    lf_set_ptr_x_absolute(winw - (width + button.padding * 2.0f) - WIN_GAP);
    lf_set_ptr_y_absolute(winh - (lf_button_dimension(text).y + button.padding * 2.0f) - WIN_GAP);

    if (lf_button_fixed(text, width, 10.0f) == LF_CLICKED && form_complete) {

        item* add = (item*)malloc(sizeof(item));
        add->tier = ratings[selected_tier];
        add->name = strdup(new_item_input_buf);

        item_entries[num_entries++] = add;

        serialize_list("./tierdata.bin");

        current_page = INIT;

        selected_tier = -1;
        opened = false;
    }

    lf_set_line_should_overflow(true);
    lf_pop_style_props();
}

void main() {

    // Initial Processing
    if (!glfwInit()) {
        fprintf(stderr, "%s", "Failed to initialize GLFW\n");
        glfwTerminate();
    }
    GLFWwindow* window = glfwCreateWindow(winw, winh, "Tier List", NULL, NULL);
    glfwMakeContextCurrent(window);
    lf_init_glfw(winw, winh, window);

    // Background config
    LfTheme theme = lf_get_theme();
    theme.div_props.color = LF_NO_COLOR;
    lf_set_theme(theme);

    configs();
    deserialize_list("./tierdata.bin");

    // Running Process
    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        lf_begin();

        // Title Config
        lf_div_begin(((vec2s){WIN_GAP, WIN_GAP}), ((vec2s){winw - (WIN_GAP * 2.0f), winh - (WIN_GAP * 2.0f)}), true);

        // Operations
        switch(current_page) {
            case INIT: {
                rendertopbar();
                rendericons();
                renderdivisions();
                renderitemcontainers();
                renderitemplacement();
                printf("Buffer address (buf): %p\n", (void*)new_item_input.buf);
                printf("Buffer size (buf_size): %u\n", new_item_input.buf_size);
                printf("Cursor index: %d\n", new_item_input.cursor_index);
                printf("Text to copy: %s\n", "abcdefghijklmnopqrstuvwxyz");
                break;
            }
            case ADD: {
                rendernewpage();
                break;
            }
        }

        lf_end();
        glfwPollEvents();
        glfwSwapBuffers(window);
    }

    // Teardown

    for (uint32_t i = 0; i < 5; i++) {
        free(ratings[i]);
        free(icon_letters[i]);
    }

    for (uint32_t i = 0; i < num_entries; i++) {
        free(item_entries[i]->name);
        free(item_entries[i]->tier);
        free(item_entries[i]);
    }

    free(ratings);
    free(icon_colors);
    free(icon_letters);
    lf_free_font(&titleFont);
    lf_free_font(&iconFont);
    lf_free_font(&itemFont);
    lf_free_font(&buttonFont);
    lf_terminate();
    glfwDestroyWindow(window);
    glfwTerminate();
}