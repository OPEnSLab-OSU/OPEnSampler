// enter in number of valves
int vn = 24;

// determine number of TPICs (8 valves per TPIC plus 1 TPIC for flushing)
int TPICcount = ceil(vn / 8) + 1;
print("The TPIC number is " TPICcount);

int testValve = 9;

//generate output bytes - 1 per TPIC