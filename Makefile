all : WTFclient WTFserver

WTFclient : threadpool.o compressProject.o serverSideMsg.o serverUpdate.o WTFserver.o checkFile.o createBeh.o updateBeh.o checkoutBeh.o cvBeh.o pushBeh.o upgradeBeh.o clientMain.o destoryBeh.o whatsMessageFromServer.o commitBeh.o getHash.o rollbackBeh.o WTFclient.o compareMani.o hBeh.o realclientmain.o mytest
	gcc -pthread -lpthread -o WTF threadpool.o compressProject.o serverSideMsg.o serverUpdate.o WTFserver.o checkFile.o createBeh.o updateBeh.o checkoutBeh.o cvBeh.o pushBeh.o upgradeBeh.o clientMain.o destoryBeh.o whatsMessageFromServer.o commitBeh.o getHash.o rollbackBeh.o WTFclient.o compareMani.o hBeh.o realclientmain.o

WTFserver : threadpool.o compressProject.o serverSideMsg.o serverUpdate.o realservermain.o WTFserver.o checkFile.o createBeh.o updateBeh.o checkoutBeh.o cvBeh.o pushBeh.o upgradeBeh.o clientMain.o destoryBeh.o whatsMessageFromServer.o commitBeh.o getHash.o rollbackBeh.o WTFclient.o compareMani.o hBeh.o
	gcc -pthread -lpthread -o WTFserver threadpool.o compressProject.o serverSideMsg.o serverUpdate.o realservermain.o WTFserver.o checkFile.o createBeh.o updateBeh.o checkoutBeh.o cvBeh.o pushBeh.o upgradeBeh.o clientMain.o destoryBeh.o whatsMessageFromServer.o commitBeh.o getHash.o rollbackBeh.o WTFclient.o compareMani.o hBeh.o

threadpool.o : threadpool.c
	gcc -c threadpool.c

creareBeh.o : createBeh.c
	gcc -c createBeh.c

upgradeBeh.o : upgradeBeh.c
	gcc -c upgradeBeh.c

checkFile.o : checkFile.c
	gcc -c checkFile.c

cvBeh.o : cvBeh.c
	gcc -c cvBeh.c

realclientmain.o : realclientmain.c
	gcc -c realclientmain.c

whatsMessageFromServer.o : whatsMessageFromServer.c
	gcc -c whatsMessageFromServer.c

checkoutBeh.o : checkoutBeh.c
	gcc -c checkoutBeh.c

destoryBeh.o : destoryBeh.c
	gcc -c destoryBeh.c

WTFclient.o : WTFclient.c
	gcc -c WTFclient.c

clientMain.o : clientMain.c
	gcc -c clientMain.c

getHash.o : getHash.c
	gcc -c getHash.c

rollbackBeh.o : rollbackBeh.c
	gcc -c rollbackBeh.c

WTFserver.o : WTFserver.c
	gcc -c WTFserver.c

commitBeh.o : commitBeh.c
	gcc -c commitBeh.c

hBeh.o : hBeh.c
	gcc -c hBeh.c

serverSideMsg.o : serverSideMsg.c
	gcc -c serverSideMsg.c

compareMani.o : compareMani.c
	gcc -c compareMani.c

serverUpdate.o : serverUpdate.c
	gcc -c serverUpdate.c

compressProject.o : compressProject.c
	gcc -c compressProject.c

pushBeh.o : pushBeh.c
	gcc -c pushBeh.c

updateBeh.o : updateBeh.c
	gcc -c updateBeh.c

realservermain.o : realservermain.c
	gcc -c realservermain.c

gethashmain.o : gethashmain.c
	gcc -c gethashmain.c

mytest : gethashmain.o
	gcc -o .testhelp gethashmain.o checkFile.o compareMani.o getHash.o

clean :
	rm WTFserver WTF WTFtest threadpool.o compressProject.o serverSideMsg.o serverUpdate.o realservermain.o WTFserver.o checkFile.o createBeh.o updateBeh.o checkoutBeh.o cvBeh.o pushBeh.o upgradeBeh.o clientMain.o destoryBeh.o whatsMessageFromServer.o commitBeh.o getHash.o rollbackBeh.o WTFclient.o compareMani.o hBeh.o realclientmain.o .testhelp gethashmain.o
	rm .Configure
	rm -rf shana2

test : test.c ./WTF .testhelp testhelp
	gcc -o WTFtest test.c


secondtest : testcheckout testcommit testpush testcheckout testhardcommit testhardpush testupdate testupgrade testupdatec testcurrentversion testrollback testhistory testclean
	@echo "\n===========Test end!================\n"
	@echo "Test finish: ALL TESTS PASSED!\n"


testhelp : testinit testdestory testcreate testcommit testpush testcheckout testhardcommit testhardpush testupdate testupgrade testupdatec testcurrentversion testrollback testhistory testclean
	@echo "\n===========Test end!================\n"
	@echo "Test finish: ALL TESTS PASSED!\n"

testcurrentversion :
	@echo "\n===========Test Currentversion Command================\n"
	./WTF currentversion shana

testhistory :
	@echo "\n===========Test History Command================\n"
	./WTF history shana


testrollback :
	@echo "\n===========Test Rollback Command================\n"
	rm -rf shana2
	mkdir shana2
	echo "0" >> shana2/.Manifest
	./WTF rollback shana 0
	diff -r shana shana2


testupdatec :
	@echo "\n===========Test Update[C Tag] Command================\n"
	rm shana/.Manifest
	mv shana2/.Manifest shana2/backup
	./.testhelp 1 ./shana/shana.txt ./shana/shana.txt true 1 1
	./.testhelp 1 ./shana/snakeOfTheFestival.txt ./shana/snakeOfTheFestival.txt false 1 1
	mv shana2/.Manifest shana/.Manifest
	mv shana2/backup shana2/.Manifest
	echo "Weapon NIETONO NO SHANA\n" >> shana/shana.txt
	./WTF update shana
	./.testhelp 7 ./shana2/shana.txt ./shana/shana.txt true
	diff shana/.Conflict shana2/.Conflict

testhardpush :
	@echo "\n===========Test Push[ADM Tag] Command================\n"
	 ./.testhelp 1 ./shana2/shana.txt ./shana/shana.txt true 2 0
	./.testhelp 1 ./shana2/snakeOfTheFestival.txt ./shana/snakeOfTheFestival.txt false 2 1
	./WTF push shana
	diff -r shana shana2

testupdate :
	@echo "\n===========Test Update[A Tag] Command================\n"
	rm shana/shana.txt
	echo "CNN. Fake news.GO AHEAD.. hehe.." >> shana/fstar.txt
	echo "shana~~~~~ haha" >> shana/shana.txt
	rm shana/snakeOfTheFestival.txt
	rm shana/.Manifest
	cp shana2/.Manifest shana2/backup
	./.testhelp 1 ./shana/fstar.txt ./shana/fstar.txt true 1 1
	./.testhelp 1 ./shana/shana.txt ./shana/shana.txt false 1 1
	mv shana2/.Manifest shana/.Manifest
	mv shana2/backup shana2/.Manifest
	./.testhelp 4 ./shana2/snakeOfTheFestival.txt ./shana/snakeOfTheFestival.txt true
	./.testhelp 5 ./shana2/shana.txt ./shana/shana.txt false
	./.testhelp 6 ./shana/fstar.txt ./shana/fstar.txt false
	./WTF update shana
	diff ./shana/.Update ./shana2/.Update

testupgrade :
	@echo "\n===========Test Upgrade Command================\n"
	rm shana2/.Update
	./WTF upgrade shana
	diff -r shana shana2

testhardcommit :
	@echo "\n===========Test Commit[ADM Tag] Command================\n"
	rm shana/intro.txt
	rm shana2/intro.txt
	echo "shana\nFire Red\n" >> shana/shana.txt
	echo "shana\nFire Red\n" >> shana2/shana.txt
	echo "Fire Black\n" >> shana/snakeOfTheFestival.txt
	echo "Fire Black\n" >> shana2/snakeOfTheFestival.txt
	./.testhelp 2 ./shana2/intro.txt ./shana/intro.txt true
	./.testhelp 3 ./shana2/snakeOfTheFestival.txt ./shana/snakeOfTheFestival.txt false
	./.testhelp 0 ./shana2/shana.txt ./shana/shana.txt false
	./WTF commit shana
	diff -r shana shana2


testcheckout :
	@echo "\n===========Test Checkout Command================\n"
	rm -rf shana
	./WTF checkout shana
	diff -r shana shana2

testinit :
	@echo "\n===========Test Configure Command================\n"
	./WTF configure 127.0.0.1 22797

testdestory :
	@echo "\n===========Test Destroy Command================\n"
	./WTF destroy shana

testcreate :
	@echo "\n===========Test Create Command================\n"
	mkdir shana2
	echo "0" >> ./shana2/.Manifest
	./WTF create shana
	diff -r shana shana2


testcommit :
	@echo "\n===========Test Commit[A Tag] Command================\n"
	echo "Flamehaze shana\n" >> shana/intro.txt
	echo "Flamehaze shana\n" >> shana2/intro.txt
	echo "Boss and yuji\n" >> shana/snakeOfTheFestival.txt
	echo "Boss and yuji\n" >> shana2/snakeOfTheFestival.txt
	./.testhelp 0 ./shana2/snakeOfTheFestival.txt ./shana/snakeOfTheFestival.txt true
	./.testhelp 0 ./shana2/intro.txt ./shana/intro.txt false
	./WTF commit shana
	diff -r shana shana2

testpush :
	@echo "\n===========Test Push Command================\n"
	./.testhelp 1 ./shana2/snakeOfTheFestival.txt ./shana/snakeOfTheFestival.txt true 1 0
	./.testhelp 1 ./shana2/intro.txt ./shana/intro.txt false 1 0
	./WTF push shana
	diff -r shana shana2

testclean :
	@echo "\n===========Remove Test File================\n"
	rm -rf shana
