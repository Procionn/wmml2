#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <filesystem>
#include <wmml.h>
#include "constants.h"
#include <iostream>

#define CATCH catch (const char* error) {std::cerr << error << std::endl;}

#define TEST_LIST								\
    X("wmml/file1.wmml", FILE_1, first_wmml)    \
    X("wmml/file2.wmml", FILE_2, second_wmml) 	\
    X("wmml/file3.wmml", FILE_3, third_wmml)  	\
    X("wmml/file4.wmml", FILE_4, fourth_wmml) 	\
    X("wmml/file5.wmml", FILE_5, fifth_wmml)	\
    X("wmml/file6.wmml", FILE_6, sixth_wmml)	\
    X("wmml/file7.wmml", FILE_7, seventh_wmml)	\
    X("wmml/file8.wmml", FILE_8, eighth_wmml)


#define X(path, vectors, name)								\
TEST(Wmml_Writing, name) { 									\
    ASSERT_NO_THROW({ 										\
		wmml file(path, vectors[0].size());					\
		for (std::vector<wmml::variant> vector : vectors) 	\
		    file.write(vector); 							\
    });														\
}
    TEST_LIST
#undef X




#define X(path, vectors, name)															\
TEST(Wmml_Reading, name) {																\
    ASSERT_NO_THROW({																	\
		wmml file(path);																\
		EXPECT_EQ(file.height(), vectors.size()) << "file height != quantity of vecors";\
		ASSERT_EQ(file.width(), vectors[0].size()) << "file width != vector wisth";		\
		std::vector<wmml::variant> vector(file.width());								\
		for (auto& object : vectors) {													\
		    file.read(vector);															\
            for (int i = 0; i != object.size(); ++i)                                    \
                ASSERT_EQ(vector[i], object[i]);                                        \
		}																				\
		EXPECT_NE((file.read(vector)), true);											\
    });																					\
}
    TEST_LIST
#undef X
#undef TEST_LIST




#define TEST_LIST											\
    X("wmml/file1.wmml", "wmml/file2.wmml", first_wmml)		\
    X("wmml/file1.wmml", "wmml/file3.wmml", second_wmml)	\
    X("wmml/file1.wmml", "wmml/file4.wmml", third_wmml)		\
    X("wmml/file5.wmml", "wmml/file1.wmml", fourth_wmml)	\
    X("wmml/file5.wmml", "wmml/file7.wmml", fifth_wmml)		\
    X("wmml/file6.wmml", "wmml/file5.wmml", sixth_wmml)		 


// 2
// 3    --> 1 -> 5 -> 6
// 4        7 


#define X(file1, file2, name)						\
TEST(Wmml_Archivation, name) {						\
	ASSERT_NO_THROW({								\
		wmml main_file(file1);						\
		main_file.set_wmml(new wmml_marker(file2));	\
	});												\
}
	TEST_LIST
#undef X
#undef TEST_LIST





void unarchivation (std::vector<const char*>& names, int& namesCounter, std::filesystem::path fileName) {
    wmml file(fileName);
    std::vector<const char*> buffer;
    while(true) {
        auto entry = file.get_wmml();
        if (!entry)
            break;
        entry->unarchiving(names[namesCounter]);
        buffer.emplace_back(names[namesCounter]);
        ++namesCounter;
    }
    for (const auto& entry : buffer)
        unarchivation(names, namesCounter, entry);
}

TEST(Wmml_DeArchivation, unarchivation) {
	std::vector<const char*> unarchived_files{"wmml/UnFile5.wmml", "wmml/UnFile7.wmml", "wmml/UnFile1.wmml", "wmml/UnFile4.wmml", "wmml/UnFile3.wmml", "wmml/UnFile2.wmml"};
	int i = 0;
	ASSERT_NO_THROW({
		unarchivation(unarchived_files, i, "wmml/file6.wmml");
	});
}






#define TEST_LIST												\
    X("wmml/file1.wmml", "wmml/UnFile1.wmml", first_wmml)		\
    X("wmml/file2.wmml", "wmml/UnFile2.wmml", second_wmml)		\
    X("wmml/file3.wmml", "wmml/UnFile3.wmml", third_wmml)		\
    X("wmml/file4.wmml", "wmml/UnFile4.wmml", fourth_wmml)		\
    X("wmml/file5.wmml", "wmml/UnFile5.wmml", fifth_wmml)		\
	X("wmml/file7.wmml", "wmml/UnFile7.wmml", seventh_wmml)

#define X(file1, file2, name)																		\
TEST(FileComparator, name) {																		\
	ASSERT_NO_THROW({																				\
		wmml oldFile(file1);																		\
		wmml newFile(file2);																		\
		ASSERT_EQ(oldFile.width(), 	newFile.width()) << "oldFile width != newFile width";			\
		EXPECT_EQ(oldFile.height(), newFile.height()) << "oldFile height != quantity of newFile"; 	\
		std::vector<wmml::variant> oldFileVector(oldFile.width());									\
		std::vector<wmml::variant> newFileVector(newFile.width());									\
		for (int c = 0, i = oldFile.height(); c != i; ++c) {										\
			oldFile.read(oldFileVector);															\
			newFile.read(newFileVector);															\
			EXPECT_EQ((oldFileVector == newFileVector), true);										\
		}																							\
	});																								\
}
	TEST_LIST
#undef X
#undef TEST_LIST





#define TEST_LIST                                                   \
    X("wmml/file1.wmml", FILE_1, 0, 1, int, 8546454, string_to_int) \
    X("wmml/file2.wmml", FILE_2, 0, 4, std::string, "this is a test overwriting to a string", string_to_string) \
    X("wmml/file3.wmml", FILE_3, 0, 0, std::string, "bool(false)", bool_to_string)      \
    X("wmml/file4.wmml", FILE_4, 0, 2, int32_t, 18446741145615, int64t_to_int32t)       \
    X("wmml/file7.wmml", FILE_7, 0, 1, int64_t, 9223372036854775807, int32t_to_int64t)

#define X(filename, data, column, string, type, value, name)    \
TEST(SectorOverwriting, name) {                                 \
    ASSERT_NO_THROW({                                           \
        wmml object(filename);                                  \
        object.overwriting_sector(column, string, type(value)); \
    });                                                         \
}
    TEST_LIST
#undef X





#define X(filename, data, column, string, type, value, name)    \
    TEST(SectorOverwritingTest, name) {                         \
        ASSERT_NO_THROW({                                       \
            auto duplicate = data;                              \
            (duplicate[column])[string] = type(value);          \
            wmml file(filename);                                \
            std::vector<wmml::variant> v(file.width());         \
            for (auto& object : duplicate) {                    \
                file.read(v);                                   \
                for (int i = 0; i != object.size(); ++i)        \
                    ASSERT_EQ(v[i], object[i]);                 \
            }                                                   \
            EXPECT_NE((file.read(v)), true);                    \
        });                                                     \
    }
    TEST_LIST
#undef X
#undef TEST_LIST





int main (int argc, char** argv) {
	std::filesystem::path dir = "wmml";
	if (std::filesystem::exists(dir))
		for (const auto& entry : std::filesystem::directory_iterator(dir))
			std::filesystem::remove_all(entry.path());
	else std::filesystem::create_directory(dir);

	::testing::InitGoogleTest(&argc, argv);

	return RUN_ALL_TESTS();
}
