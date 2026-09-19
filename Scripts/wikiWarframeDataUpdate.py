import requests
from pathlib import Path


# URL for Warframe Wiki API
apiURL = "https://wiki.warframe.com/api.php"

# MediaWiki policy custom User-Agent
headers = {
    "User-Agent": "warframeDamageOptimizationCalculator/DataRefresh/1.0 (https://github.com/CalebNoss/WarframeDamageOptimizer)"
}

# token
payload = {
    "token": "+\\"
}

# List of API requests for Primary, Secondary, Melee, Mods, and Arcanes database requests from the wiki
apiEndpoints = [
    "https://wiki.warframe.com/api.php?action=scribunto-console&format=json&title=Test&content=&question=%3Drequire(%22Module%3AJSON%22).stringify(mw.loadData(%22Module%3AWeapons%2Fdata%2Fprimary%22))&clear=1&formatversion=2",
    "https://wiki.warframe.com/api.php?action=scribunto-console&format=json&title=Test&content=&question=%3Drequire(%22Module%3AJSON%22).stringify(mw.loadData(%22Module%3AWeapons%2Fdata%2Fsecondary%22))&clear=1&formatversion=2",
    "https://wiki.warframe.com/api.php?action=scribunto-console&format=json&title=Test&content=&question=%3Drequire(%22Module%3AJSON%22).stringify(mw.loadData(%22Module%3AWeapons%2Fdata%2Fmelee%22))&clear=1&formatversion=2",
    "https://wiki.warframe.com/api.php?action=scribunto-console&format=json&title=Test&content=&question=%3Drequire(%22Module%3AJSON%22).stringify(mw.loadData(%22Module%3AMods%2Fdata%22))&clear=1&formatversion=2",
    "https://wiki.warframe.com/api.php?action=scribunto-console&format=json&title=Test&content=&question=%3Drequire(%22Module%3AJSON%22).stringify(mw.loadData(%22Module%3AArcane%2Fdata%22))&clear=1&formatversion=2"
]
# made using: https://wiki.warframe.com/w/Special:ApiSandbox#action=scribunto-console&format=json&title=Test&content=&question=%3Drequire(%22Module%3AJSON%22).stringify(mw.loadData(%22Module%3AArcane%2Fdata%22))&clear=1&token=%2B%5C&formatversion=2
# Special API Sandbox/Request generator

# list of paths for output of API requests to go
outputFiles = [
    "wikiData/wikiExportPrimary.json",
    "wikiData/wikiExportSecondary.json",
    "wikiData/wikiExportMelee.json",
    "wikiData/wikiExportMods.json",
    "wikiData/wikiExportArcanes.json"
]

# anchor to script location instead of moving where scripts go based on where you execute it from
pythonScriptDirectory = Path(__file__).resolve().parent

# token is +\\ (techniccally I think it should just be +\, but it needs to be +\\ for the escape character)

for apiURL, outputFileName in zip(apiEndpoints, outputFiles):
    try:
        # Send API request using POST method
        response = requests.post(apiURL, headers=headers, data=payload)

        outputFile = pythonScriptDirectory.parent / outputFileName

        # if successful
        if response.status_code == 200:
            jsonString = response.text

            with open(outputFile, "w") as file:
                file.write(jsonString)
        else:
            print(f"Error fetching data from {apiURL}, the status code is: {response.status_code}")
    except requests.exceptions.RequestException as e:
        print(f"Error fetching data from {apiURL}, it says: {e}")
print("I got the new data! It should all be written to file too.")