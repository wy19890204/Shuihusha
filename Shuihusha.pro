# -------------------------------------------------
# Project created by QtCreator 2010-06-13T04:26:52
# -------------------------------------------------
TARGET = Shuihusha
QT += network sql declarative
TEMPLATE = app
CONFIG += warn_on audio
CONFIG += use_rcc

# If you want to enable joystick support, please uncomment the following line:
# CONFIG += joystick
# However, joystick is not supported under Mac OS X temporarily

# If you want enable voice reading for chat content, uncomment the following line:
# CONFIG += chatvoice
# Also, this function can only enabled under Windows system as it make use of Microsoft TTS

SOURCES += \
	src/main.cpp \
	src/client/aux-skills.cpp \
	src/client/client.cpp \
	src/client/clientplayer.cpp \
	src/client/clientstruct.cpp \
	src/core/banpair.cpp \
	src/core/crypto.cpp \
	src/core/card.cpp \
	src/core/engine.cpp \
	src/core/exppattern.cpp \
	src/core/general.cpp \
	src/core/jsonutils.cpp \
	src/core/lua-wrapper.cpp \
	src/core/player.cpp \
	src/core/protocol.cpp \
	src/core/record-analysis.cpp \
	src/core/settings.cpp \
	src/core/skill.cpp \
	src/core/statistics.cpp \
	src/core/util.cpp \
	#src/core/wrapped_card.cpp \
	src/dialog/cardeditor.cpp \
	src/dialog/cardoverview.cpp \
	src/dialog/choosecarddialog.cpp \
	src/dialog/choosegeneraldialog.cpp \
	src/dialog/configdialog.cpp \
	src/dialog/connectiondialog.cpp \
	src/dialog/customassigndialog.cpp \
	src/dialog/distanceviewdialog.cpp \
	src/dialog/generaloverview.cpp \
	src/dialog/mainwindow.cpp \
	src/dialog/packagingeditor.cpp \
	src/dialog/playercarddialog.cpp \
	src/dialog/roleassigndialog.cpp \
	src/dialog/scenario-overview.cpp \
	src/dialog/halldialog.cpp \
	src/package/package.cpp \
	src/package/standard-cards.cpp \
	src/package/standard-generals.cpp \
	src/package/common-skillcards.cpp \
	src/package/standard.cpp \
	src/package/plough.cpp \
	src/package/maneuvering.cpp \
	src/package/events.cpp \
	src/package/gift.cpp \
	src/package/rat.cpp \
	src/package/ox.cpp \
	src/package/tiger.cpp \
	src/package/hare.cpp \
	src/package/dragon.cpp \
	src/package/snake.cpp \
	src/package/mustang.cpp \
	src/package/sheep.cpp \
	src/package/monkey.cpp \
	src/package/cock.cpp \
	src/package/boar.cpp \
	src/package/sp.cpp \
	src/package/mini-generals.cpp \
	src/package/joy.cpp \
	src/package/guben.cpp \
	src/scenario/contract.cpp \
	src/scenario/couple.cpp \
	src/scenario/dusong.cpp \
	src/scenario/changban.cpp \
	src/scenario/impasse_fight.cpp \
	src/scenario/miniscenarios.cpp \
	src/scenario/landlord.cpp \
	src/scenario/legend.cpp \
	src/scenario/scenario.cpp \
	src/scenario/scenerule.cpp \
	src/scenario/warlords.cpp \
	src/scenario/wheel_fight.cpp \
	src/scenario/zombie.cpp \
	src/server/ai.cpp \
	src/server/contestdb.cpp \
	src/server/gamerule.cpp \
	src/server/generalselector.cpp \
	src/server/room.cpp \
	src/server/roomthread.cpp \
	src/server/roomthread1v1.cpp \
	src/server/roomthread3v3.cpp \
	src/server/server.cpp \
	src/server/serverplayer.cpp \
	src/ui/button.cpp \
	src/ui/cardcontainer.cpp \
	src/ui/carditem.cpp \
	src/ui/chatwidget.cpp \
	src/ui/clientlogbox.cpp \
	src/ui/dashboard.cpp \
	src/ui/indicatoritem.cpp \
	src/ui/irregularbutton.cpp \
	src/ui/photo.cpp \
	src/ui/pixmap.cpp \
	src/ui/pixmapanimation.cpp \
	src/ui/rolecombobox.cpp \
	src/ui/roomscene.cpp \
	src/ui/sprite.cpp \
	src/ui/startscene.cpp \
	src/ui/window.cpp \
	src/util/detector.cpp \
	src/util/nativesocket.cpp \
	src/util/recorder.cpp \
	src/lua/print.c \
	src/lua/lzio.c \
	src/lua/lvm.c \
	src/lua/lundump.c \
	src/lua/ltm.c \
	src/lua/ltablib.c \
	src/lua/ltable.c \
	src/lua/lstrlib.c \
	src/lua/lstring.c \
	src/lua/lstate.c \
	src/lua/lparser.c \
	src/lua/loslib.c \
	src/lua/lopcodes.c \
	src/lua/lobject.c \
	src/lua/loadlib.c \
	src/lua/lmem.c \
	src/lua/lmathlib.c \
	src/lua/llex.c \
	src/lua/liolib.c \
	src/lua/linit.c \
	src/lua/lgc.c \
	src/lua/lfunc.c \
	src/lua/ldump.c \
	src/lua/ldo.c \
	src/lua/ldebug.c \
	src/lua/ldblib.c \
	src/lua/lcode.c \
	src/lua/lbaselib.c \
	src/lua/lauxlib.c \
	src/lua/lapi.c \
	src/jsoncpp/src/json_writer.cpp \
	src/jsoncpp/src/json_valueiterator.inl \
	src/jsoncpp/src/json_value.cpp \
	src/jsoncpp/src/json_reader.cpp \
	src/jsoncpp/src/json_internalmap.inl \
	src/jsoncpp/src/json_internalarray.inl \
	swig/sanguosha_wrap.cxx

HEADERS += \
	src/client/aux-skills.h \
	src/client/client.h \
	src/client/clientplayer.h \
	src/client/clientstruct.h \
	src/core/audio.h \
	src/core/banpair.h \
	src/core/crypto.h \
	src/core/card.h \
	src/core/engine.h \
	src/core/exppattern.h \
	src/core/general.h \
	src/core/jsonutils.h \
	src/core/lua-wrapper.h \
	src/core/player.h \
	src/core/protocol.h \
	src/core/record-analysis.h \
	src/core/settings.h \
	src/core/skill.h \
	src/core/statistics.h \
	src/core/structs.h \
	src/core/util.h \
	#src/core/wrapped_card.h \
	src/dialog/cardeditor.h \
	src/dialog/cardoverview.h \
	src/dialog/choosecarddialog.h \
	src/dialog/choosegeneraldialog.h \
	src/dialog/configdialog.h \
	src/dialog/connectiondialog.h \
	src/dialog/customassigndialog.h \
	src/dialog/distanceviewdialog.h \
	src/dialog/generaloverview.h \
	src/dialog/halldialog.h \
	src/dialog/mainwindow.h \
	src/dialog/packagingeditor.h \
	src/dialog/playercarddialog.h \
	src/dialog/roleassigndialog.h \ 
	src/dialog/scenario-overview.h \
	src/package/package.h \
	src/package/standard-equips.h \
	src/package/standard-generals.h \
	src/package/common-skillcards.h \
	src/package/standard.h \
	src/package/plough.h \
	src/package/maneuvering.h \
	src/package/events.h \
	src/package/gift.h \
	src/package/rat.h \
	src/package/ox.h \
	src/package/tiger.h \
	src/package/hare.h \
	src/package/dragon.h \
	src/package/snake.h \
	src/package/mustang.h \
	src/package/sheep.h \
	src/package/monkey.h \
	src/package/cock.h \
	src/package/boar.h \
	src/package/sp.h \
	src/package/mini-generals.h \
	src/package/joy.h \
	src/package/guben.h \
	src/scenario/contract.h \
	src/scenario/couple.h \
	src/scenario/dusong.h \
	src/scenario/changban.h \
	src/scenario/impasse_fight.h \
	src/scenario/miniscenarios.h \
	src/scenario/landlord.h \
	src/scenario/legend.h \
	src/scenario/scenario.h \
	src/scenario/scenerule.h \
	src/scenario/warlords.h \
	src/scenario/wheel_fight.h \
	src/scenario/zombie.h \
	src/server/ai.h \
	src/server/contestdb.h \
	src/server/gamerule.h \
	src/server/generalselector.h \
	src/server/room.h \
	src/server/roomthread.h \
	src/server/roomthread1v1.h \
	src/server/roomthread3v3.h \
	src/server/server.h \
	src/server/serverplayer.h \
	src/ui/button.h \
	src/ui/cardcontainer.h \
	src/ui/carditem.h \
	src/ui/chatwidget.h \
	src/ui/clientlogbox.h \
	src/ui/dashboard.h \
	src/ui/indicatoritem.h \
	src/ui/irregularbutton.h \
	src/ui/photo.h \
	src/ui/pixmap.h \
	src/ui/pixmapanimation.h \
	src/ui/rolecombobox.h \
	src/ui/roomscene.h \
	src/ui/sprite.h \
	src/ui/startscene.h \
	src/ui/window.h \
	src/util/detector.h \
	src/util/nativesocket.h \
	src/util/recorder.h \
	src/util/socket.h \
	src/lua/lzio.h \
	src/lua/lvm.h \
	src/lua/lundump.h \
	src/lua/lualib.h \
	src/lua/luaconf.h \
	src/lua/lua.hpp \
	src/lua/lua.h \
	src/lua/ltm.h \
	src/lua/ltable.h \
	src/lua/lstring.h \
	src/lua/lstate.h \
	src/lua/lparser.h \
	src/lua/lopcodes.h \
	src/lua/lobject.h \
	src/lua/lmem.h \
	src/lua/llimits.h \
	src/lua/llex.h \
	src/lua/lgc.h \
	src/lua/lfunc.h \
	src/lua/ldo.h \
	src/lua/ldebug.h \
	src/lua/lcode.h \
	src/lua/lauxlib.h \
	src/lua/lapi.h \
	src/jsoncpp/src/json_tool.h \
	src/jsoncpp/src/json_batchallocator.h \
	src/jsoncpp/include/json/writer.h \
	src/jsoncpp/include/json/value.h \
	src/jsoncpp/include/json/reader.h \
	src/jsoncpp/include/json/json.h \
	src/jsoncpp/include/json/forwards.h \
	src/jsoncpp/include/json/features.h \
	src/jsoncpp/include/json/config.h \
	src/jsoncpp/include/json/autolink.h \
	src/jsoncpp/include/json/assertions.h
	
FORMS += \
	src/dialog/cardoverview.ui \
	src/dialog/configdialog.ui \
	src/dialog/connectiondialog.ui \
	src/dialog/generaloverview.ui \
	src/dialog/mainwindow.ui 

INCLUDEPATH += include
INCLUDEPATH += src/client
INCLUDEPATH += src/core
INCLUDEPATH += src/dialog
INCLUDEPATH += src/package
INCLUDEPATH += src/scenario
INCLUDEPATH += src/server
INCLUDEPATH += src/ui
INCLUDEPATH += src/util
INCLUDEPATH += src/lua
INCLUDEPATH += src/jsoncpp/include

LIBS += -Llib -lcryptopp
LIBS += -L.

TRANSLATIONS += shuihusha.ts

OTHER_FILES += \
        shuihusha.qss \
        acknowledgement/main.qml \
        swig/sanguosha.i \
        swig/qvariant.i \
        swig/naturalvar.i \
        swig/native.i \
        swig/luaskills.i \
        swig/list.i \
        swig/card.i \
        swig/ai.i \
        etc/3v3-priority.txt \
        etc/1v1-priority.txt \
        etc/customScenes/30.txt \
        etc/customScenes/29.txt \
        etc/customScenes/28.txt \
        etc/customScenes/27.txt \
        etc/customScenes/26.txt \
        etc/customScenes/25.txt \
        etc/customScenes/24.txt \
        etc/customScenes/23.txt \
        etc/customScenes/22.txt \
        etc/customScenes/21.txt \
        etc/customScenes/20.txt \
        etc/customScenes/19.txt \
        etc/customScenes/18.txt \
        etc/customScenes/17.txt \
        etc/customScenes/16.txt \
        etc/customScenes/15.txt \
        etc/customScenes/14.txt \
        etc/customScenes/13.txt \
        etc/customScenes/12.txt \
        etc/customScenes/11.txt \
        etc/customScenes/10.txt \
        etc/customScenes/09.txt \
        etc/customScenes/08.txt \
        etc/customScenes/07.txt \
        etc/customScenes/06.txt \
        etc/customScenes/05.txt \
        etc/customScenes/04.txt \
        etc/customScenes/03.txt \
        etc/customScenes/02.txt \
        etc/customScenes/01.txt \
        image/system/coord_normal.ini \
        image/system/coord_circular.ini \
        image/system/dashboard.ini \
        image/system/photo.ini \
        lua/config.lua

win32{
        RC_FILE += resource/icon.rc
}

macx{
        ICON = resource/icon/sgs.icns
}

CONFIG(audio){
	DEFINES += AUDIO_SUPPORT
	INCLUDEPATH += include/fmod
	LIBS += -lfmodex
	SOURCES += src/core/audio.cpp
}

CONFIG(joystick){
	DEFINES += JOYSTICK_SUPPORT
	HEADERS += src/ui/joystick.h
	SOURCES += src/ui/joystick.cpp
	win32: LIBS += -lplibjs -lplibul -lwinmm
	unix: LIBS += -lplibjs -lplibul
}

CONFIG(chatvoice){
	win32{
		CONFIG += qaxcontainer
		DEFINES += CHAT_VOICE
	}
}

CONFIG(use_rcc){
        DEFINES += USE_RCC
        RESOURCES += \
		image/skin.qrc \
		image/system/button/irregular.qrc
}

CONFIG(qrc){
RESOURCES += \
    image/big-card.qrc \
    image/card.qrc \
    image/system/emotion/weapon.qrc \
    image/system/emotion/success.qrc \
    image/system/emotion/peach.qrc \
    image/system/emotion/no-success.qrc \
    image/system/emotion/revive.qrc \
    image/system/emotion/death.qrc \
    image/system/emotion/damage.qrc \
    image/system/emotion/damage2.qrc \
    image/system/emotion/chain.qrc \
    image/system/emotion/armor.qrc \
    image/system/emotion/analeptic.qrc \
    image/system/emotion/tsunami.qrc \
    image/system/emotion/hplost.qrc \
    image/system/emotion/awake.qrc \
    image/system/emotion/assassinate.qrc \
    image/system/emotion/thunder_damage.qrc \
    image/system/emotion/fire_damage.qrc \
    image/system/emotion/avoid.qrc \
    image/system/emotion/judgegood.qrc \
    image/system/emotion/judgebad.qrc \
    image/system/emotion/horse.qrc \
    image/system/emotion/draw-card.qrc \
    image/system/emotion/lightning.qrc \
    image/system/emotion/killer.qrc \
    image/system/emotion/jink.qrc \
    image/system/emotion/recover.qrc \
    image/system/emotion/limited.qrc \
    image/system/emotion/duel.qrc \
    image/system/emotion/thunder_slash.qrc \
    image/system/emotion/slash_red.qrc \
    image/system/emotion/slash_black.qrc \
    image/system/emotion/fire_slash.qrc \
    image/system/emotion/pindian.qrc \
    image/system/emotion/god_salvation.qrc \
    image/system/emotion/amazing_grace.qrc \
    image/system/emotion/archery_attack.qrc \
    image/system/emotion/inspiration.qrc \
    backdrop/shuihu-cover.qrc \
    backdrop/shuihu.qrc
}
