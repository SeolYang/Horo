 #include <catch2/catch_session.hpp>

 int main(int argc, char* argv[])
{
     int result = Catch::Session().run(argc, argv);
     system("PAUSE");
     return result;
 }