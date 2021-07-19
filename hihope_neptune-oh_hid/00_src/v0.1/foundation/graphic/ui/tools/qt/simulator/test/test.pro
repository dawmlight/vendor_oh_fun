CONFIG -= qt

TEMPLATE = lib
CONFIG += staticlib

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

DEFINES += QT_COMPILER

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

DESTDIR = ../libs

SOURCES += \
    ../../../../test/framework/src/test_case_list_adapter.cpp \
    ../../../../test/framework/src/ui_test_group.cpp \
    ../../../../test/framework/src/test_ability.cpp \
    ../../../../test/framework/src/ui_test_app.cpp \
    ../../../../test/uitest/test_animator/ui_test_animator.cpp \
    ../../../../test/uitest/test_anti_aliasing/ui_test_anti_aliasing.cpp \
    ../../../../test/uitest/test_arc_label/ui_test_arc_label.cpp \
    ../../../../test/uitest/test_button/ui_test_button.cpp \
    ../../../../test/uitest/test_canvas/ui_test_canvas.cpp \
    ../../../../test/uitest/test_chart/ui_test_chart_pillar.cpp \
    ../../../../test/uitest/test_chart/ui_test_chart_polyline.cpp \
    ../../../../test/uitest/test_dialog/ui_test_dialog.cpp \
    ../../../../test/uitest/test_digital_clock/ui_test_digital_clock.cpp \
    ../../../../test/uitest/test_draw_line/ui_test_draw_line.cpp \
    ../../../../test/uitest/test_draw_rect/ui_test_draw_rect.cpp \
    ../../../../test/uitest/test_event_injector/ui_test_event_injector.cpp \
    ../../../../test/uitest/test_font/ui_test_font.cpp \
    ../../../../test/uitest/test_image/ui_test_image.cpp \
    ../../../../test/uitest/test_image_animator/ui_test_image_animator.cpp \
    ../../../../test/uitest/test_input_event/ui_test_input_event.cpp \
    ../../../../test/uitest/test_label/ui_test_label.cpp \
    ../../../../test/uitest/test_layout/ui_test_advanced_layout.cpp \
    ../../../../test/uitest/test_layout/ui_test_basic_layout.cpp \
    ../../../../test/uitest/test_opacity/ui_test_opacity.cpp \
    ../../../../test/uitest/test_picker/ui_test_ui_picker.cpp \
    ../../../../test/uitest/test_progress_bar/ui_test_box_progress.cpp \
    ../../../../test/uitest/test_progress_bar/ui_test_circle_progress.cpp \
    ../../../../test/uitest/test_qrcode/ui_test_qrcode.cpp \
    ../../../../test/uitest/test_render/ui_test_render.cpp \
    ../../../../test/uitest/test_rotate_input/ui_test_rotate_input.cpp \
    ../../../../test/uitest/test_slider/ui_test_slider.cpp \
    ../../../../test/uitest/test_texture_mapper/ui_test_texture_mapper.cpp \
    ../../../../test/uitest/test_transform/ui_test_transform.cpp \
    ../../../../test/uitest/test_ui_analog_clock/ui_test_analog_clock.cpp \
    ../../../../test/uitest/test_ui_dump_dom_tree/ui_test_dump_dom.cpp \
    ../../../../test/uitest/test_ui_list/ui_test_ui_list.cpp \
    ../../../../test/uitest/test_ui_list_view/ui_test_list_layout.cpp \
    ../../../../test/uitest/test_ui_scroll_view/ui_test_ui_scroll_view.cpp \
    ../../../../test/uitest/test_ui_swipe_view/ui_test_ui_swipe_view.cpp \
    ../../../../test/uitest/test_vector_font/ui_test_vector_font.cpp \
    ../../../../test/uitest/test_view_percent/ui_test_view_percent.cpp \
    ../../../../test/uitest/test_view_scale_rotate/ui_test_view_scale_rotate.cpp

HEADERS += \
    ../../../../test/framework/include/test_case_list_adapter.h \
    ../../../../test/framework/include/ui_test_app.h \
    ../../../../test/framework/include/test_resource_config.h \
    ../../../../test/framework/include/ui_test.h \
    ../../../../test/framework/include/ui_test_group.h \
    ../../../../test/uitest/test_animator/ui_test_animator.h \
    ../../../../test/uitest/test_anti_aliasing/ui_test_anti_aliasing.h \
    ../../../../test/uitest/test_arc_label/ui_test_arc_label.h \
    ../../../../test/uitest/test_button/ui_test_button.h \
    ../../../../test/uitest/test_canvas/ui_test_canvas.h \
    ../../../../test/uitest/test_chart/ui_test_chart_pillar.h \
    ../../../../test/uitest/test_chart/ui_test_chart_polyline.h \
    ../../../../test/uitest/test_dialog/ui_test_dialog.h \
    ../../../../test/uitest/test_digital_clock/ui_test_digital_clock.h \
    ../../../../test/uitest/test_draw_line/ui_test_draw_line.h \
    ../../../../test/uitest/test_draw_rect/ui_test_draw_rect.h \
    ../../../../test/uitest/test_event_injector/ui_test_event_injector.h \
    ../../../../test/uitest/test_font/ui_test_font.h \
    ../../../../test/uitest/test_image/ui_test_image.h \
    ../../../../test/uitest/test_image_animator/ui_test_image_animator.h \
    ../../../../test/uitest/test_input_event/ui_test_input_event.h \
    ../../../../test/uitest/test_label/ui_test_label.h \
    ../../../../test/uitest/test_layout/ui_test_advanced_layout.h \
    ../../../../test/uitest/test_layout/ui_test_basic_layout.h \
    ../../../../test/uitest/test_opacity/ui_test_opacity.h \
    ../../../../test/uitest/test_picker/ui_test_ui_picker.h \
    ../../../../test/uitest/test_progress_bar/ui_test_box_progress.h \
    ../../../../test/uitest/test_progress_bar/ui_test_circle_progress.h \
    ../../../../test/uitest/test_qrcode/ui_test_qrcode.cpp \
    ../../../../test/uitest/test_render/ui_test_render.h \
    ../../../../test/uitest/test_rotate_input/ui_test_rotate_input.h \
    ../../../../test/uitest/test_slider/ui_test_slider.h \
    ../../../../test/uitest/test_texture_mapper/ui_test_texture_mapper.h \
    ../../../../test/uitest/test_transform/ui_test_transform.h \
    ../../../../test/uitest/test_ui_analog_clock/ui_test_analog_clock.h \
    ../../../../test/uitest/test_ui_dump_dom_tree/ui_test_dump_dom.h \
    ../../../../test/uitest/test_ui_list/ui_test_ui_list.h \
    ../../../../test/uitest/test_ui_list_view/ui_test_list_layout.h \
    ../../../../test/uitest/test_ui_scroll_view/ui_test_ui_scroll_view.h \
    ../../../../test/uitest/test_ui_swipe_view/ui_test_ui_swipe_view.h \
    ../../../../test/uitest/test_vector_font/ui_test_vector_font.h \
    ../../../../test/uitest/test_view_percent/ui_test_view_percent.h \
    ../../../../test/uitest/test_view_scale_rotate/ui_test_view_scale_rotate.h

INCLUDEPATH += \
    ../../../../frameworks \
    ../../../../../utils/frameworks/windows \
    ../../../../../utils/interfaces/innerkits \
    ../../../../../utils/interfaces/kits \
    ../../../../../ui/interfaces/frameworks \
    ../../../../../ui/interfaces/innerkits \
    ../../../../../ui/interfaces/kits \
    ../../../../test/framework/include \
    ../../../../test/uitest \
    ../../../../../../../third_party/bounds_checking_function/include \
    ../../../../../../../third_party/harfbuzz/src \
    ../../../../../../../third_party/icu/icu4c/source/common \
    ../../../../../../../third_party/freetype/include \
    ../../../../../../../third_party/cJSON \
    ../../../../../../../third_party/libjpeg \
    ../../../../../../../third_party/libpng
