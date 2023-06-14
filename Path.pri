
CONFIG(release, debug|release) {
    BuildType=release
    CONFIG += warn_off
} else {
    BuildType=debug
    DEFINES += __DEBUG
}

msvc {
    QMAKE_CFLAGS += /utf-8
    QMAKE_CXXFLAGS += /utf-8
}

#DESTDIR         = $$PWD/../RunImage/$$BuildType
DESTDIR = $$absolute_path($$PWD/bin/)



