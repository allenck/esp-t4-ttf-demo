# Edit following two lines to set component requirements (see docs)
set(COMPONENT_REQUIRES )
set(COMPONENT_PRIV_REQUIRES )


idf_component_register(
	SRCS
	"main.c"
	"unicode.c"
	"hagl_util.c"
	INCLUDE_DIRS
	"."
	EMBED_TXTFILES "Ubuntu-R.ttf"
)
set(COMPONENT_ADD_INCLUDEDIRS "")

#register_component()
