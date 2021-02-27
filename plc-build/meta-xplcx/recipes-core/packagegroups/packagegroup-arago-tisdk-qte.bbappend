
PR_append = "-e"

QT5_DEMOS_remove = "\
    qt3d-examples \
    qtbase-examples \
    qtdeclarative-examples \
    qtdeclarative-tools \
    qtlocation-examples \
    qtmultimedia-examples \
    qtscript-examples \
    qtsvg-examples \
    ${@bb.utils.contains('DISTRO_FEATURES', 'wayland', 'qtwayland-examples', '', d)} \
    qtwebkit-examples-examples \
    qtserialport-examples \
    qtcharts-examples \
"

QT_DEMOS_remove = "\
    qt-tstat \
"
