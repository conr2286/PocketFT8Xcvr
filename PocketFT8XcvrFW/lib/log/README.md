# Amateur Radio log library

The log library consists of serveral components:
* Contact:  collects information about a contact (QSO)
* ADIFlog:  Records a Contact in an ADIF log
* CSVlog:   Records a Contact in a CSV log
* LogFactory:  Builds an appropriate (e.g. ADIFlog) log object
* FileSystemAdapter:  Somewhat generic adapter to the host's file system

## Contact
Contact determines what information can be collected about a QSO.  The chosen items
have a definite ADIF accent but do not determine the use of ADIF logging.

## ADIFlog
This is where a Contact's collected information is recorded to an ADIF 
encoded log file.

## LogFactory
The factory enables an application to choose ADIF vs CSV logging at runtime.
