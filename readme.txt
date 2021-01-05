               WTF SERVER AND WTF CLIENT

NAME 
    WTF : client of this version control system
    WTFserver: server of this version control system


SYNOPSIS
    ./WTFserver : run server on 127.0.0.1 : 9999 by default
                  (might need to change port on ilab)
    ./WTF [command] [arg1] [arg2] : run client to connect the server
                                    ip and port on .Configure file


DSCRIPTION
    version control system


FINISHED COMMAND
    configure : save ip and port into .Configure file
    create : create a new project on the server, also mkdir on the client
    checkout : download project from the server
    update : check if there are any updates on the server
    upgrade : download new updagtes from the server
    commit : check local file, see if any updates
    push : push updates to server
    destroy : remove a project on the server and client
    currentversion : check current version of a given project
    history : check history of all the actions(push/rollback)
    rollback : roll back to a version


STRUCT 
    FOR MULTI-THREADING 
    [1]struct threadnode : 
        args for sub-threading's arg, also is a node of linked list.
        contains clientSocket, pthread_t, threadnode* next ,
        and int(boolean) isDone
    [2]struct threadpool :
        linked list container for threadnode
        save all sub-threading info, free source when get SIGINT signal

    FOR PATH/FILE NAME
    [1]struct FilePath :
        save file name and file path from a string
        "FILEPATH/FILENAME" => struct FilePath {file_name, file_path}
        contains file name and file path
    [2]struct fileName : 
        take out a file name from a string
        contains file name and current position(index) in string(char[])

    FOR FILE CONTENT
    [1]struct Manifest :
        a linked list for saving .Manifest/.Update/.Comit file
        contains int size, Node *head, and int mver(currentVersion/0/0)
                 
    [2]struct Node :
        save a entry from a line in the file(.Manifest/.Update/.Comit) 
        entry must be XXX:XXXXX:XXX\n
        contains char* filename(fpath/fname), char *version, char *hash
        and Node *next
        version : numberOfVersion/operationCode/operationCode
        hash might be 0 as placeholder


DESIGN 
    FOR MULTI-THREADING & MUTEX
    [1]When server get accept a new client,
       lock mutex and save this client to a temporary variable(threadnode) 
       Then pass temporary variable to sub-threading function. 
       Unlock mutex in this sub-threading function.
    [2]Once new thread is created, put new threadnode into threadpool.
       When server down/close, iterate threadpool, close clients(sub-thread)
    [3]when a sub-threading finish its job, 
       will set isDone = 1(in threadnode) which means it can be free/close
    [4]Once we need to write any thing on the server,
       we will lock mutex for that threading and file.
    [5]When locking a mutex, 
       we make sure there are no locked mutex inside of locked part
       and still locked outside.
       means we did this order : lockA -> lockB -> unlockB -> unlockA
       instead of lockA -> lockB _> unlockA -> unlockB
       (b locked inside of lockA -> unlockA, and still locked after unlockA)

    FOR NETWORK COMMUNICATION
    [1]message format :
       #msg head#content#DONE#
       when client/server meet #DONE#, 
       the client/server know they read all the message from server/client
    [2]message head : 
        msg head(client to server) : 
            CT(create), CK(checkout), UD(update), UG(upgrade),
            CM(commit), PS(push), CV(currentversion), HS(history),
            DS(destroy), RB(rollback)
        msg head(server to client) :
            CTR(create), CKR(checkout), UDR(update), UGR(upgrade),
            CMR(commit), PSR(push), CVR(currentversion), HSR(history),
            DSR(destroy), RBR(rollback)
            some of response msg head was not used(like DSR)
            becasue we just send error/success instead of msg head.
    [3]error/succes/fail message :
        #ERR_1# : project already existed on the server
        #ERR_2# : no this project on the server 
        #ERR_3# : project is up-to-date
        #ERR_4# : update needed
        #ERR_8# : no this version for a given project
        #FAI_1# : destroy project failed
        #SUC_1# : destroy project successful
    [4]content :
        Basically, all the message was set by the follow format:
            number_of_files#
            number_of_filename#filename#number_content_in_file#content_file
            ......
            ......
        But, for some command(like history and currentversion)
        we didn't use this message format.
        for example, content for history is just content in .History file

    FOR FILE READING AND WRITING
    [1]write special files in the same format(.Manifest/.Update/.Commit...)
        for this kind of file, we use the same format as follow:
            version\n 
            versionForFile:filename:hashcode\n 
            .......
        In .Update/.Commit file, the first line will be 0\n, and 
        versionForFile will be operationCode([A],[D],[M],[C]), and
        hash will be set as 0 if not necessary.
        In .Manifest file, version and versionForFile are integer, and
        hashcode is hash for the file's content
    [2]compare files
        we only need to compare files which is in the format talked above
        other user files will directly over-write if hash code is not same
        so when comparing .Manifest and .Commit file in "push"
        we convert these file into a linked list(struct Manifest)
        save the version, and each entries.
        then compare two linked list instead of comparing two file directly
    [3]rewrite files
        for special files, create a new Manifest linked list
        iterate old list(s), put entries which we are interested in new list
        for other use files just re-write everything if hash is not same
    [4]hash function
        we use BKDR hash function to hash string in the file

    FOR VERSION CONTROL SYSTEM
    [1]backup old version
        when server start, 
        it will create a directory ".BackupLib" if it not existed.
            for example after server running
            create .BackupLib => ./BackupLib
        when a project is created,
        it also generate a dir in .BackupLib and .History in that dir.
            for example after running ./WTF create newproject
            create newproject in ./
            create newproject in .BackupLib => ./.BackupLib/newproject
            create .Histroy => ./.BackupLib/newproject/.History
        when a push action success
        it will create a compressed file into .BackupLib 
        using currentversion as the file name
        then update all the files on the server side
        write action into .History file
            for example after running ./WTF push newproject
            compress newproject => ./.BackupLib/newproject/1
            (if current version is 1)
            
