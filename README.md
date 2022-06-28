# bxdiff
bxdiff and enough of my lib to build it.

bxdiff is Brian's xdiff.  It is setup to work fine on a black and white display.  Does anybody still have one?  So far, no editing just less like browsing.

usage examples: more or less replace diff with bxdiff...

```
    bxdiff file1 file2 
    bxdiff -wd file1 dir
```

Once the viewer is up:

   * j/DOWN - line down
   * SPACE/PAGE_DOWN - page down
   * k/UP - line up
   * b/PAGE_UP - page up
   * g/HOME - goto beginning
   * G/END - goto end
   * n - goto next diff region
   * p - goto previous diff region
   * / - start new regexp find [RET-to perform search, BS-to fix it]
   * N/RIGHT - goto next regexp match
   * P/LEFT - goto previous regexp match
   * q - quits
   * Click the mouse on the right hand side map to go there.
