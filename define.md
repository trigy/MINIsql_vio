# the define
## index
 block zero include data as:  
    4        2     4      
 maxOffset  type  root   
 other:  
   1       1        2      2      4      4    type       4  
 valid  nodeType  width  store  parent  next  [key offsetInFile]  
  
 width=((4096-13)/(n+4))-1 for split  
  
## record
 block zero include data as:  
    4            4  
 maxOffset lengthPerRecord  
 other:  
   1       8            2     2             length   
 valid null bitmap  [pos1,length1]   ...    [data] ...  
 
 recordPerBlock=4096/lengthPerRecord  
 blockOffset=(offset/recordPerBlock)+1  
 offsetInBlock=offset%recordPerBlock  
  
## catalog
### table(just one block)
   2        2         8             2    2      2        length   
attNum  primaryNum unique bitmap  [pos,length,type] ...  [name] ...  
for every attribute, the length is <=20, and one block has <=32 attributes
### index(just one block after table)
   2         20         20
indexNum  [indexName, keyName] ...  
