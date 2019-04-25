#!/usr/bin/env python

#day of week at start of month : [date of first sunday,date of last sunday]
# (sunday = 0)
truth = { 0: [1,29],
          1: [7,28],
          2: [6,27],
          3: [5,26],
          4: [4,25],
          5: [3,31],
          6: [2,30] }

dates_in_month = range(1,32,1) #1-31

days_of_week = range(7) #0-6 ... sunday = 0


for day_of_week in days_of_week:
    for date in dates_in_month:


        #find the day of the week # for the 1st of the month, given only current date and day of week.:
        DOWonthefirst = ((6+day_of_week-date%7)%7+2)%7 #i know it's ugly but it works.

        #determine mday of last Sunday
        n = date
        n = n - day_of_week
        n = n + 7
        d = n % 7  #date of first Sunday
        if d==0:
          d=7
        
        if truth[DOWonthefirst][0]!=d:
            print("FIRSTMISMATCH:","date:", date, "day_of_week:" , day_of_week, "DOWonthefirst:", DOWonthefirst, "truth:", truth[DOWonthefirst][0], "d:",d)
           
        n = 31 - d
        n = int(n/7) #number of Sundays left in the month
        d = d + 7 * n #mday of final Sunday


        if truth[DOWonthefirst][1] !=d:
            print("MISMATCH:", "date:", date, "day_of_week", day_of_week, "day_of_week on the first:", DOWonthefirst, "truth:", truth[DOWonthefirst][1], "d:", d)

