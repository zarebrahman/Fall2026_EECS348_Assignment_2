# EECS 348 Assignment 2

## Build

```bash
make
```

## Run with the sample file

```bash
./assignment2 sample_test.txt
```

The program also accepts redirected input:

```bash
./assignment2 < sample_test.txt
```

## Compare the sample output

```bash
./assignment2 sample_test.txt > actual_output.txt
diff -u sample_expected.txt actual_output.txt
```

No output from `diff` means the result matches the provided sample.

## Files Required Before Submission

- `assignment2.c` - final source code
- `assignment2` - executable compiled on the Cycle server
- `GenAI_Analysis.pdf` - final comparison report
- `GENAI_PROMPT.txt` - exact prompt used for both GenAI programs
- `ChatGPT_raw.c` and `Gemini_raw.c` - unedited comparison programs
- `Makefile` - build instructions
- Test files used to check the program

## Final Cycle Server Check

Run these commands from this folder before the final GitHub push:

```bash
make clean
make
./assignment2 sample_test.txt > actual_output.txt
diff -u sample_expected.txt actual_output.txt
rm actual_output.txt
```

If `diff` prints nothing, the sample output matches. The `assignment2`
executable produced by this step is the Cycle-server executable that must be
committed to GitHub.

The repository must be named `Fall2026_EECS348_Assignment_2`, and this folder must remain named `3173804_Rahman_Assignment_2`.
