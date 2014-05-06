# My Stuff:
# OpenCV
win32{
    INCLUDEPATH *= C:/Temp/OpenCV/opencv/build/include
    LIBS *= -L"C:/Temp/OpenCV/opencv/build/x64/vc11/lib"
    Debug:LIBS *= -lopencv_core248d -lopencv_highgui248d -lopencv_features2d248d -lopencv_nonfree248d -lopencv_flann248d -lopencv_imgproc248d
    Release:LIBS *= -lopencv_core248 -lopencv_highgui248 -lopencv_features2d248 -lopencv_nonfree248 -lopencv_flann248 -lopencv_imgproc248
}

# OpenMP
win32{
    QMAKE_CXXFLAGS *= /openmp
    QMAKE_CXXFLAGS *= /MP
}
unix:!mac{
    QMAKE_CXXFLAGS *= -fopenmp
    LIBS += -lgomp
}

# Other
win32{
    DEFINES *= NOMINMAX _USE_MATH_DEFINES
}

# Build flag
CONFIG(debug, debug|release) {
    CFG = debug
} else {
    CFG = release
}
