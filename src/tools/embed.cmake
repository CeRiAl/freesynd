add_executable (embed tools/embed.c)
function (embed_data input name output)
	add_custom_command (
		OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${output}
		COMMAND $<TARGET_FILE:embed> ${CMAKE_CURRENT_SOURCE_DIR}/${input} ${name} ${output}
		DEPENDS embed
		MAIN_DEPENDENCY ${input}
	)
endfunction ()
