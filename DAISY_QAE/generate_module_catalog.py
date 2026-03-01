import os
import re
import glob

def parse_ts_file(filepath):
    with open(filepath, 'r', encoding='utf-8') as f:
        content = f.read()

    # Extract basic info
    display_name_match = re.search(r"displayName:\s*['\"](.*?)['\"]", content)
    display_name = display_name_match.group(1) if display_name_match else "Unknown"

    category_match = re.search(r"category:\s*BlockCategory\.([A-Z_]+)", content)
    category = category_match.group(1) if category_match else "Unknown"

    description_match = re.search(r"description:\s*['\"](.*?)['\"]", content)
    description = description_match.group(1) if description_match else ""

    # Extract Ports
    ports = []
    # Simple regex to find port blocks. This might be fragile but let's try.
    # We look for objects inside ports: [ ... ]
    # A port usually looks like: { id: ..., displayName: '...', ... direction: ... }
    
    # Let's find the ports array content first
    ports_array_match = re.search(r"ports:\s*\[(.*?)\]", content, re.DOTALL)
    if ports_array_match:
        ports_content = ports_array_match.group(1)
        # Split by objects approximately
        # This is hard with regex. Let's just scan for displayNames and directions
        # Assume they appear in pairs
        
        # Better approach: find all port definitions blocks
        port_pattern = r"\{\s*id:.*?displayName:\s*['\"](.*?)['\"].*?direction:\s*PortDirection\.([A-Z_]+).*?\}"
        port_matches = re.finditer(port_pattern, ports_content, re.DOTALL)
        for pm in port_matches:
            p_name = pm.group(1)
            p_dir = pm.group(2) # INPUT or OUTPUT
            ports.append(f"{p_name} ({p_dir})")
            
    # Extract Parameters
    params = []
    params_array_match = re.search(r"parameters:\s*\[(.*?)\]", content, re.DOTALL)
    if params_array_match:
        params_content = params_array_match.group(1)
        # Find param definitions
        param_pattern = r"\{\s*id:.*?displayName:\s*['\"](.*?)['\"].*?type:\s*ParameterType\.([A-Z_]+).*?\}"
        param_matches = re.finditer(param_pattern, params_content, re.DOTALL)
        for pm in param_matches:
            p_name = pm.group(1)
            p_type = pm.group(2)
            params.append(f"{p_name}")

    if not ports:
        # Retry with wider regex if first failed (maybe formatting differences)
        port_pattern_loose = r"displayName:\s*['\"](.*?)['\"].*?direction:\s*PortDirection\.([A-Z_]+)"
        if ports_array_match:
             port_matches = re.finditer(port_pattern_loose, ports_array_match.group(1), re.DOTALL)
             ports = [f"{m.group(1)} ({m.group(2)})" for m in port_matches]

    return {
        "Name": display_name,
        "Category": category,
        "Description": description,
        "Inputs": [p.replace(" (INPUT)", "") for p in ports if "INPUT" in p],
        "Outputs": [p.replace(" (OUTPUT)", "") for p in ports if "OUTPUT" in p],
        "Controls": params
    }

def main():
    root_dir = "src/core/blocks/definitions"
    files = glob.glob(os.path.join(root_dir, "*.ts"))
    files.extend(glob.glob(os.path.join(root_dir, "utility", "*.ts")))
    files.sort()

    print("# DVPE Module Catalog\n")
    print(f"**Total Modules**: {len(files)}\n")
    
    # Group by category
    modules_by_category = {}
    
    for fp in files:
        if "_template" in fp: continue
        module = parse_ts_file(fp)
        cat = module["Category"]
        if cat not in modules_by_category:
            modules_by_category[cat] = []
        modules_by_category[cat].append(module)

    for cat in sorted(modules_by_category.keys()):
        print(f"## {cat}\n")
        print("| Module | Description | Inputs | Outputs | Controls |")
        print("|---|---|---|---|---|")
        for mod in modules_by_category[cat]:
            inputs = "<br>".join(mod["Inputs"]) if mod["Inputs"] else "-"
            outputs = "<br>".join(mod["Outputs"]) if mod["Outputs"] else "-"
            controls = "<br>".join(mod["Controls"]) if mod["Controls"] else "-"
            print(f"| **{mod['Name']}** | {mod['Description']} | {inputs} | {outputs} | {controls} |")
        print("\n")

if __name__ == "__main__":
    main()
