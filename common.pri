# My Stuff:
# OpenCV
win32{
    INCLUDEPATH *= C:/Development/opencv/build/include
    LIBS *= -L"C:/Development/opencv/build/x64/vc11/lib"
    Debug:LIBS *= -lopencv_core249d -lopencv_highgui249d -lopencv_features2d249d -lopencv_nonfree249d -lopencv_flann249d -lopencv_imgproc249d
    Release:LIBS *= -lopencv_core249 -lopencv_highgui249 -lopencv_features2d249 -lopencv_nonfree249 -lopencv_flann249 -lopencv_imgproc249
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
