#!/usr/bin/env python3

#day of week at start of month : [date of first sunday,date of last sunday]
# (sunday = 0)

truth = { 0: [0,28],
          1: [6,27],
          2: [5,26],
          3: [4,25],
          4: [3,24],
          5: [2,30],
          6: [1,29] }

dates_in_month = range(31) #0-30

days_of_week = range(7) #0-6 ... sunday = 0


for day_of_week in days_of_week:
    for date in dates_in_month:

        #find the day of the week # for the 1st of the month, given only current date and day of week.:
        DOWonthefirst = (day_of_week-date)%7

        #determine mday of last Sunday
        n = date
        n = n - day_of_week
        n = n + 7
        d = n % 7  #date of first Sunday
        
        if truth[DOWonthefirst][0]!=d:
            print("FIRSTMISMATCH:","date:", date, "day_of_week:" , day_of_week, "DOWonthefirst:", DOWonthefirst, "truth:", truth[DOWonthefirst][0], "d:",d)
           
        n = 31 - d
        n = int(n/7) #number of Sundays left in the month
        d = d + 7 * n #mday of final Sunday

#        if d == 31: #Dirty fix...
#        	d=24


        if truth[DOWonthefirst][1] !=d:
            print("LASTMISMATCH:", "date:", date, "day_of_week", day_of_week, "day_of_week on the first:", DOWonthefirst, "truth:", truth[DOWonthefirst][1], "d:", d)

