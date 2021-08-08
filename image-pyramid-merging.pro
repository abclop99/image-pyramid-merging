QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    selectFiles.cpp

HEADERS += \
    mainwindow.h

FORMS += \
    mainwindow.ui

INCLUDEPATH += C:\tools\OpenCV-3.2.0\opencv\build\include

LIBS += C:\tools\OpenCV-3.2.0\opencv-build\bin\libopencv_core320.dll
LIBS += C:\tools\OpenCV-3.2.0\opencv-build\bin\libopencv_highgui320.dll
LIBS += C:\tools\OpenCV-3.2.0\opencv-build\bin\libopencv_imgcodecs320.dll
LIBS += C:\tools\OpenCV-3.2.0\opencv-build\bin\libopencv_imgproc320.dll
LIBS += C:\tools\OpenCV-3.2.0\opencv-build\bin\libopencv_features2d320.dll
LIBS += C:\tools\OpenCV-3.2.0\opencv-build\bin\libopencv_calib3d320.dll

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    apple.jpg \
    orange.jpg

RESOURCES += \
    resources.qrc
