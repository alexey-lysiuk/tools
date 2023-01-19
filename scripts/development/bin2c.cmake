cmake_minimum_required(VERSION 3.4)  # needed for string(APPEND)

function(bin2c INPUT_FILE VARIABLE_NAME OUTPUT_FILE)
	#message("bin2c(\"${INPUT_FILE}\" \"${VARIABLE_NAME}\" \"${OUTPUT_FILE}\")")

	file(READ "${INPUT_FILE}" INPUT_HEX HEX)
	#message("INPUT_HEX = \"${INPUT_HEX}\"")

	string(LENGTH ${INPUT_HEX} HEX_LENGTH)
	#message("HEX_LENGTH = ${HEX_LENGTH}")

	set(HEX_POS 0)
	set(OUTPUT_STRING "// clang-format off\nconst unsigned char ${VARIABLE_NAME}[] = {")

	while(${HEX_POS} LESS ${HEX_LENGTH})
		math(EXPR CONTINUE_LINE "${HEX_POS} % 20")
		if(NOT CONTINUE_LINE)
			string(APPEND OUTPUT_STRING "\n")
		endif()

		string(SUBSTRING ${INPUT_HEX} ${HEX_POS} 2 HEX_BYTE)
		string(TOUPPER ${HEX_BYTE} HEX_BYTE)
		string(APPEND OUTPUT_STRING "0x${HEX_BYTE}, ")

		math(EXPR HEX_POS "${HEX_POS} + 2")
	endwhile()

	math(EXPR BYTE_LENGTH "${HEX_LENGTH} / 2")
	string(APPEND OUTPUT_STRING "\n};\nconst int ${VARIABLE_NAME}_size = ${BYTE_LENGTH};\n")
	#message("OUTPUT_STRING = \"${OUTPUT_STRING}\"")

	file(WRITE "${OUTPUT_FILE}" "${OUTPUT_STRING}")
endfunction()

if (${CMAKE_ARGC} LESS 7)
	message("Usage: cmake -P bin2c.cmake -- input_file variable_name output_file")
else()
	bin2c(${CMAKE_ARGV4} ${CMAKE_ARGV5} ${CMAKE_ARGV6})
endif()
