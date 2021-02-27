
PR_append = "-e"

QT4_ESSENTIALS_remove = "\
    qt4-embedded \
    qt4-embedded-plugin-mousedriver-tslib \
    qt4-embedded-plugin-gfxdriver-gfxtransformed \
    qt4-embedded-plugin-phonon-backend-gstreamer \
    qt4-embedded-plugin-imageformat-gif \
    qt4-embedded-plugin-imageformat-jpeg \
    qt4-embedded-qml-plugins \
    libqt-embeddedmultimedia4 \
    libqt-embeddeddeclarative4 \
    libqt-embeddedxmlpatterns4 \
    ${@oe.utils.conditional('ARAGO_QT_PROVIDER', 'qt4-embedded-gles', '${QT4_SGX_SUPPORT}', '', d)} \
"

QT5_ESSENTIALS_remove = "\
    qt3d-qmlplugins \
    qtwebkit-qmlplugins \
    qtlocation-plugins \
    qtlocation-qmlplugins \
    qtmultimedia-qmlplugins \
	\
    qtconnectivity \
    qtconnectivity-qmlplugins \
"

QT5_ESSENTIALS += "\
	qtserialbus \
	qtvirtualkeyboard \
"
