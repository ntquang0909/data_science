#CXX := g++

CPPFLAGS = -I$(HOME)/CocCoc/glog-0.4.0/include
CPPFLAGS += -I$(HOME)/CocCoc/googletest-release-1.8.1/include

LDLIBS = -L/usr/lib/x86_64-linux-gnu -pthread

LDLIBS_TS = $(HOME)/CocCoc/glog-0.4.0/lib/libglog.a
LDLIBS_TS += $(HOME)/CocCoc/googletest-release-1.8.1/lib/libgtest.a
LDLIBS_TS += $(HOME)/CocCoc/googletest-release-1.8.1/lib/libgtest_main.a

OBJS_SERVER = eventmanager.o tcp.o server.o
OBJS_CLIENT = eventmanager.o tcp.o client.o
OBJS_EVENTMANAGER = eventmanager.o eventmanager_test.o
OBJS_IOBUFFER = iobuffer_test.o
OBJS_TCP = eventmanager.o tcp.o tcp_test.o
TARGET_SERVER = server
TARGET_CLIENT = client
TARGET_EVENTMANAGER = eventmanager_test
TARGET_IOBUFFER = iobuffer_test
TARGET_TCP = tcp_test

server: $(TARGET_SERVER)

$(TARGET_SERVER): $(OBJS_SERVER)
	$(CXX) $(LDLIBS) -o $@ $(OBJS_SERVER) $(LDLIBS_TS)

$(OBJS_SERVER): %.o: %.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $<

client: $(TARGET_CLIENT)

$(TARGET_CLIENT): $(OBJS_CLIENT)
	$(CXX) $(LDLIBS) -o $@ $(OBJS_CLIENT) $(LDLIBS_TS)

$(OBJS_CLIENT): %.o: %.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $<

eventmanager_test: $(TARGET_EVENTMANAGER)

$(TARGET_EVENTMANAGER): $(OBJS_EVENTMANAGER)
	$(CXX) $(LDLIBS) -o $@ $(OBJS_EVENTMANAGER) $(LDLIBS_TS)

$(OBJS_EVENTMANAGER): %.o: %.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $<

iobuffer_test: $(TARGET_IOBUFFER)

$(TARGET_IOBUFFER): $(OBJS_IOBUFFER)
	$(CXX) $(LDLIBS) -o $@ $(OBJS_IOBUFFER) $(LDLIBS_TS)

$(OBJS_IOBUFFER): %.o: %.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $<

tcp_test: $(TARGET_TCP)

$(TARGET_TCP): $(OBJS_TCP)
	$(CXX) $(LDLIBS) -o $@ $(OBJS_TCP) $(LDLIBS_TS)

$(OBJS_TCP): %.o: %.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $<

clean:
	$(RM) $(OBJS_SERVER) $(OBJS_CLIENT) $(OBJS_EVENTMANAGER) $(OBJS_IOBUFFER) $(OBJS_TCP) $(TARGET_SERVER) $(TARGET_CLIENT) $(TARGET_EVENTMANAGER) $(TARGET_IOBUFFER) $(TARGET_TCP) *~

eventmanager_valgrind:
	valgrind -v --track-origins=yes --leak-check=full --show-reachable=yes ./eventmanager_test --gtest_repeat=100 --gtest_break_on_failure

iobuffer_valgrind:
	valgrind -v --track-origins=yes --leak-check=full --show-reachable=yes ./iobuffer_test --gtest_repeat=100 --gtest_break_on_failure

tcp_valgrind:
	valgrind -v --track-origins=yes --leak-check=full --show-reachable=yes ./tcp_test --gtest_repeat=100 --gtest_break_on_failure

