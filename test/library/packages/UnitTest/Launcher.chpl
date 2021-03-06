/*
  This launcher will be used to run the test files and catch the halts.
  You can set whether or not to descend recursively into 
  subdirectories (defaults to true) using `--subdir`
*/
module Launcher {
  use FileSystem;
  use Spawn;
  use Path;
  use TestResult;
  use Sys;
  use Help;
  config const subdir = true;
  config const keepExec = false;
  config const setComm: string = "";
  var comm: string;

  /* Gets the comm */
  proc getRuntimeComm() throws {
    var line: string;
    var checkComm = spawn(["python",CHPL_HOME:string+"/util/chplenv/chpl_comm.py"],
                        stdout = PIPE);
    while checkComm.stdout.readline(line) {
      comm = line.strip();
    }
    // setting communication mechanism.
    if setComm != "" {
      if comm != "none" {
        comm = setComm;
      }
      else {
        if setComm == "none" then comm = setComm;
        else {
          writeln("Trying to execute in a multiLocale environment when ",
          "communication mechanism is `none`.");
          writeln("Try changing the communication mechanism");
          exit(2);
        }
      }
    }
  }


  proc main(args: [] string) {
    var comm_c: c_string;
    var dirs: [1..0] string,
        files: [1..0] string;
    var hadInvalidFile = false;
    var programName = args[0];
    try! {
      var checkChpl = spawn(["which","chpl"],stdout = PIPE);
      checkChpl.wait();
      var line: string;
      if checkChpl.stdout.readline(line) {
        // get comm
        getRuntimeComm();

        for a in args[1..] {
          if a == "-h" || a == "--help" {
            writeln("Usage: ", programName, " <options> filename [filenames] directoryname [directorynames]");
            printUsage();
            exit(0);
          }
          else {
            try! {
              if isFile(a) {
                files.push_back(a);
              }
              else if isDir(a) {
                dirs.push_back(a);
              }
              else {
                writeln("[Error: ", a, " is not a valid file or directory]");
                hadInvalidFile = true;
              }
            }
          }
        }

        if hadInvalidFile && files.size == 0 && dirs.size == 0 {
          exit(2);
        }

        if files.size == 0 && dirs.size == 0 {
          dirs.push_back(".");
        }
        
        var result =  new TestResult();

        for tests in files {
          try {
            testFile(tests, result);
          }
          catch e {
            writeln("Caught an Exception in Running Test File: ", tests);
            writeln(e);
          }
        }

        for dir in dirs {
          try {
            testDirectory(dir, result);
          }
          catch e {
            writeln("Caught an Exception in Running Test Directory: ", dir);
            writeln(e);
          }
        }
          
        result.printErrors();
        writeln(result.separator2);
        result.printResult();
      }
      else {
        writeln("chpl not found.");
      } 
    }
    
  }
  
  pragma "no doc"
  /*Docs: Todo*/
  proc testFile(file, ref result) throws {
    var fileName = basename(file);
    if fileName.endsWith(".chpl") {
      var line: string;
      var compErr = false;
      var tempName = fileName.split(".chpl");
      var executable = tempName[1];
      var executableReal = executable + "_real";
      if isFile(executable) {
        FileSystem.remove(executable);
      }
      if isFile(executableReal) {
        FileSystem.remove(executableReal);
      }
      var sub = spawn(["chpl", file, "-o", executable, "-M.", 
                      "--comm", comm], stderr = PIPE); //Compiling the file
      var compError: string;
      if sub.stderr.readline(line) {
        compError = line;
        while sub.stderr.readline(line) do compError += line;
        compErr = true;
      }
      sub.wait();
      if !compErr {
        var testNames: [1..0] string,
            failedTestNames: [1..0] string,
            erroredTestNames: [1..0] string,
            testsPassed: [1..0] string,
            skippedTestNames: [1..0] string;
        var dictDomain: domain(int);
        var dict: [dictDomain] int;
        runAndLog(executable, fileName, result, numLocales, testsPassed,
                  testNames, dictDomain, dict, failedTestNames, erroredTestNames,
                  skippedTestNames);
        if !keepExec {
          FileSystem.remove(executable);
          if isFile(executableReal) {
            FileSystem.remove(executableReal);
          }
        }
      }
      else {
        writeln("Compilation Error in ", fileName);
        writeln("Possible Reasons can be passing a non-test ",
                "function to UnitTest.runTest()");
        writeln(compError);
      }
    }
  }

  pragma "no doc"
  /*Docs: Todo*/
  proc testDirectory(dir, ref result) throws {
    for file in findfiles(startdir = dir, recursive = subdir) {
      testFile(file, result);
    }
  }

  pragma "no doc"
  /*Docs: Todo*/
  proc runAndLog(executable, fileName, ref result, reqNumLocales: int = numLocales,
                ref testsPassed, ref testNames, ref dictDomain, ref dict, 
                ref failedTestNames, ref erroredTestNames, ref skippedTestNames) throws 
  {
    var separator1 = result.separator1,
        separator2 = result.separator2;
    var flavour: string,
        line: string,
        testExecMsg: string;
    var reqLocales = 0;
    var sep1Found = false,
        haltOccured = false;
    var testNamesStr,
        failedTestNamesStr,
        erroredTestNamesStr,
        passedTestStr,
        skippedTestNamesStr = "None";

    var currentRunningTests: [1..0] string;
    if testNames.size != 0 then testNamesStr = testNames: string;
    if failedTestNames.size != 0 then failedTestNamesStr = failedTestNames: string;
    if erroredTestNames.size != 0 then erroredTestNamesStr = erroredTestNames: string;
    if testsPassed.size != 0 then passedTestStr = testsPassed: string;
    if skippedTestNames.size != 0 then skippedTestNamesStr = skippedTestNames: string;
    var exec = spawn(["./"+executable, "-nl", reqNumLocales: string, "--testNames", 
              testNamesStr,"--failedTestNames", failedTestNamesStr, "--errorTestNames", 
              erroredTestNamesStr, "--ranTests", passedTestStr, "--skippedTestNames", 
              skippedTestNamesStr], stdout = PIPE, 
              stderr = PIPE); //Executing the file
    //std output pipe
    while exec.stdout.readline(line) {
      if line.strip() == separator1 then sep1Found = true;
      else if line.strip() == separator2 && sep1Found {
        var testName = currentRunningTests.pop_back();
        var checkStatus = testNames.find(testName);
        if checkStatus[1] then
          testNames.remove(checkStatus[2]);
        addTestResult(result, dictDomain, dict, testNames, flavour, fileName, 
                  testName, testExecMsg, failedTestNames, erroredTestNames, 
                  skippedTestNames, testsPassed);
        testExecMsg = "";
        sep1Found = false;
      }
      else if line.startsWith("Flavour") {
        var temp = line.strip().split(":");
        flavour = temp[2].strip();
        testExecMsg = "";
      }
      else if sep1Found then testExecMsg += line;
      else {
        if line.strip().endsWith(")") {
          var testName = line.strip();
          var checkStatus = currentRunningTests.find(testName);
          if !checkStatus[1] {
            currentRunningTests.push_back(testName);
            checkStatus = testNames.find(testName);
            if !checkStatus[1] {
              testNames.push_back(testName);
            }
          }
          testExecMsg = "";
        }  
      }
    }
    //this is to check the error
    if exec.stderr.readline(line) { 
      var testErrMsg = line;
      while exec.stderr.readline(line) do testErrMsg += line;
      if !currentRunningTests.isEmpty() {
        var testNameIndex = currentRunningTests.pop_back();
        var testName = testNameIndex;
        var checkStatus = testNames.find(testName);
        if checkStatus[1] {
          testNames.remove(checkStatus[2]);
        }
        erroredTestNames.push_back(testName);
        result.addError(testName, fileName, testErrMsg);
        haltOccured =  true;
      }
    }
    exec.wait();//wait till the subprocess is complete
    if haltOccured then
      runAndLog(executable, fileName, result, reqNumLocales, testsPassed,
                testNames, dictDomain, dict, failedTestNames, erroredTestNames,
                skippedTestNames);
    if testNames.size != 0 {
      var maxCount = -1;
      for key in dictDomain.sorted() {
        if maxCount < dict[key] {
          reqLocales = key;
          maxCount = dict[key];
        }
      }
      dictDomain.remove(reqLocales);
      runAndLog(executable, fileName, result, reqLocales, testsPassed,
                testNames, dictDomain, dict, failedTestNames, erroredTestNames, 
                skippedTestNames);
    }
  }

  pragma "no doc"
  /*Docs: Todo*/
  proc addTestResult(ref result, ref dictDomain, ref dict, ref testNames, 
                    flavour, fileName, testName, errMsg, ref failedTestNames, 
                    ref erroredTestNames, ref skippedTestNames, ref testsPassed) throws 
  {
    select flavour {
      when "OK" {
        result.addSuccess(testName, fileName);
        testsPassed.push_back(testName);
      }
      when "ERROR" {
        result.addError(testName, fileName, errMsg);
        erroredTestNames.push_back(testName);
      }
      when "FAIL" {
        result.addFailure(testName, fileName, errMsg);
        failedTestNames.push_back(testName);
      }
      when "SKIPPED" {
        result.addSkip(testName, fileName, errMsg);
        skippedTestNames.push_back(testName);
      }
      when "IncorrectNumLocales" {
        if comm != "none" {
          var strSplit = errMsg.split("=");
          var reqLocalesStr = strSplit[2].strip().split(" ");
          for a in reqLocalesStr do
            if dictDomain.contains(a: int) then
              dict[a: int] += 1;
            else
              dict[a: int] = 1;
          testNames.push_back(testName);
        }
        else {
          var locErrMsg = "Not a MultiLocale Environment. $CHPL_COMM = " + comm + "\n";
          locErrMsg += errMsg; 
          result.addSkip(testName, fileName, locErrMsg);
          skippedTestNames.push_back(testName);
        }
      }
      when "Dependence" {
        testNames.push_back(testName);
      }
    }
  }
}
