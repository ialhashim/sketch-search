# My Stuff:
# OpenCV
win32{
    INCLUDEPATH *= C:/Development/opencv/build/include
    LIBS *= -L"C:/Development/opencv/build/x64/vc11/lib"
    OpenCV_VERSION = 248
    Debug:LIBS *= -lopencv_core$${OpenCV_VERSION}d -lopencv_highgui$${OpenCV_VERSION}d -lopencv_features2d$${OpenCV_VERSION}d -lopencv_nonfree$${OpenCV_VERSION}d -lopencv_flann$${OpenCV_VERSION}d -lopencv_imgproc$${OpenCV_VERSION}d
    Release:LIBS *= -lopencv_core$${OpenCV_VERSION} -lopencv_highgui$${OpenCV_VERSION} -lopencv_features2d$${OpenCV_VERSION} -lopencv_nonfree$${OpenCV_VERSION} -lopencv_flann$${OpenCV_VERSION} -lopencv_imgproc$${OpenCV_VERSION}
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
