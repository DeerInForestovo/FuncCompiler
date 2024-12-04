myif a b c = case a of True -> b
                       False -> c

handle_odd a = collatz_conjecture(3 * a + 1)

handle_even a = collatz_conjecture(div a 2)

collatz_conjecture a = myif (a == 1) [1] ([a] ++ (myif (mod a 2 == 0) handle_even handle_odd) a)

main = do
    putStrLn (show (collatz_conjecture 323))
