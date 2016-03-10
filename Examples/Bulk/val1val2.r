/******************************************************************************
   Note; value1 is _NOT_ needed in this aggregate statement.  In fact, it makes
   this program run _much_ slower than it should.  I've put it here to test
   some aspects of the performance on large amounts of data.
******************************************************************************/

aggregate(value1, value2);

foreach(value2:) {
    printf("count(%f)=%5d\n", value2, count(value2));
}
