import google.generativeai as genai
import os
import sys
import pathlib


PROC_STATS_FILE = "/proc/netfilter_scanner/stats"
PROC_BLOCKED_FILE = "/proc/netfilter_scanner/blocked_ips"

def get_netfilter_data():
    """Reads the data from the proc files."""
    stats_data = "No stats file found."
    blocked_data = "No blocked_ips file found."

    
    try:
        stats_data = pathlib.Path(PROC_STATS_FILE).read_text()
    except Exception as e:
        stats_data = f"Error reading stats: {e}. (Is the kernel module loaded?)"
    
    try:
        blocked_data = pathlib.Path(PROC_BLOCKED_FILE).read_text()
    except Exception as e:
        blocked_data = f"Error reading blocked_ips: {e}. (Is the kernel module loaded?)"

    return stats_data, blocked_data

def build_prompt(user_question, stats, blocked_ips):
    return f"""
    You are a helpful Linux network security assistant.
    Your job is to answer an administrator's questions in plain English.
    
    You will be given the raw data from the 'netfilter_smart_port_scan_detector' kernel module.
    
    ---
    CONTEXT: CURRENT SYSTEM STATUS
    
    File: {PROC_STATS_FILE}
    {stats}
    
    File: {PROC_BLOCKED_FILE}
    {blocked_ips}
    ---
    
    ADMINISTRATOR'S QUESTION:
    "{user_question}"
    
    Based *only* on the context above, answer the question.
    If the data is not available, say so.
    """

def main():
    
    api_key = os.getenv("GOOGLE_API_KEY")
    if not api_key:
        print("Error: GOOGLE_API_KEY environment variable not set.")
        print("Please set it with: export GOOGLE_API_KEY=\"YOUR_API_KEY_HERE\"")
        sys.exit(1)
    
    genai.configure(api_key=api_key)
    
    
    if len(sys.argv) < 2:
        print("Usage: python3 ask_netfilter.py \"Your question here\"")
        sys.exit(1)
    
    user_question = " ".join(sys.argv[1:])

    print(f"Querying... (Asking: '{user_question}')\n")
    
    
    stats_data, blocked_data = get_netfilter_data()
    
    
    prompt = build_prompt(user_question, stats_data, blocked_data)
    
    
    try:
        
        model = genai.GenerativeModel('gemini-2.5-flash')
        response = model.generate_content(prompt)
        
        
        print("---Assistant Response---")
        print(response.text)
        print("---------------------------")
        
    except Exception as e:
        print(f"\nAn error occurred while calling the Gemini API: {e}")

if __name__ == "__main__":
    main()