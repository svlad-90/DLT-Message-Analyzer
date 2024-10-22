[**Go to the previous page**](../../README.md)

----

# Table of Contents

- [Coverage note](#coverage-note)
  - [Use-case](#use-case)
  - [How does it look?](#how-does-it-look)
- [Working with coverage note items](#working-with-coverage-note-items)
  - [Adding the coverage note item](#adding-the-coverage-note-item)
    - [From the search view](#from-the-search-view)
      - [Adding the coverage note item from the main table](#adding-the-coverage-note-item-from-the-main-table)
      - [Adding the coverage note item from the search view](#adding-the-coverage-note-item-from-the-search-view)
    - [From the coverage note view](#from-the-coverage-note-view)
      - [Adding the coverage note item from the main table](#adding-the-coverage-note-item-from-the-main-table-1)
      - [Adding the 'general' coverage note item](#adding-the-general-coverage-note-item)
      - [Adding the 'files data' coverage note item](#adding-the-files-data-coverage-note-item)
  - [Deleting the coverage note item](#deleting-the-coverage-note-item)
  - [Moving the coverage note item](#moving-the-coverage-note-item)
    - [Moving the coverage note item up](#moving-the-coverage-note-item-up)
    - [Moving the coverage note item down](#moving-the-coverage-note-item-down)
- [Working with the coverage note](#working-with-the-coverage-note)
  - [Creating a new coverage note](#creating-a-new-coverage-note)
  - [Opening the coverage note](#opening-the-coverage-note)
  - [Saving the coverage note](#saving-the-coverage-note)
    - [Save](#save)
    - [Save as](#save-as)
  - [Exporting the coverage note as HTML](#exporting-the-coverage-note-as-html)

# Coverage note

## Use-case

I want to create a coverage note within the plugin as a developer.

That is needed to:

- decrease ticket processing time
- increase the coverage note data readability
- increase the coverage note data reusability. For example, I want to be able to quickly apply all search queries of some previously analyzed ticket to a new data set
- increase the level of the coverage note data structure

It should be possible to:

- Add a coverage note item from the "search view" to the coverage note:
  - The coverage note item should contain messages selected in the "search view" of the plugin.
  - The coverage note item should also include the regular expression used for the search.
  - The messages should be stored in this coverage note item with the HTML highlighting.
  - It should be possible to trigger a search using the regex stored in this coverage note item.
- Add a coverage note item from the main dlt-viewer's table to the coverage note:
  - The coverage note item should contain messages selected in the main dlt-viewer's table.
- Add a 'general' coverage note item:
  - Adding a generic coverage note item that is not tied to specific messages should be possible.
- Add a 'used files' coverage note item:
  - It should be possible to add a coverage note item that describes which DLT files were used during the analysis.
- Provide a user's comment for each stored coverage note item.
- Store the 'creation date', 'username', 'messages', 'comment', and 'regex' fields for each created coverage note item.
- Store the coverage note as a JSON file.
- Open previously saved coverage note files.
- Change the order of the added coverage note items.
- Delete and edit existing coverage note items.
- Export the coverage note to HTML so that it can be viewed by users not using the DLT viewer.
- Observe the name of the currently opened coverage note file.
- Provide the username to be used for the created coverage note items.

All the functionality described above is made possible by the DLT Message Analyzer plugin.

## How does it look?

The coverage note functionality is provided by:

- The "coverage note view" tab of the plugin
- The "search view" tab of the plugin

![Screenshot of the "Coverage note" appearance](./coverage_note.jpeg)

----

## Working with coverage note items

### Adding the coverage note item

You can add new coverage note items to the coverage note in several ways.

#### From the search view

##### Adding the coverage note item from the main table

- Open any \*.dlt file and select some messages in the main dlt-viewer's table:

  ![Screenshot of the selected messages within the main dlt-viewer's table](./main_dlt_viewer_table_selected_messages.jpeg)

- Switch to the "search view" and use the context menu or 'Ctrl+Alt+M' shortcut to add new coverage note item:

  ![Screenshot of the context menu in the search view that allows you to add the coverage note item to the coverage note from the main dlt-viewer's table](./search_view_add_main_table_coverage_note_item_context_menu.jpeg)
  
- You will be switched to the 'coverage note view'. You can edit your newly added coverage note item there:

  ![Screenshot of editing the coverage note coverage note item obtained from the main table](./editing_coverage_note_from_the_main_table.jpeg)

##### Adding the coverage note item from the search view

- Open any \*.dlt file, start the search in the DLT Message Analyzer, and select some of the messages that you want to make part of the new coverage note item:

  ![Screenshot of search result with selected several lines and opened context menu](./add_search_result_coverage_note_item.jpeg)

- You will be switched to the 'coverage note view'. You can edit your newly added coverage note item there:

  ![Screenshot of editing the coverage note coverage note item obtained from the search result](./editing_coverage_note_from_the_search_result.jpeg)

  **Note!** The search result messages are stored with the preserved highlighting.
  
- The regex field will be filled in for such cases. Using the saved regular expression, you can trigger a search using the "Apply regex" button.

#### From the coverage note view

##### Adding the coverage note item from the main table

Apply the same steps mentioned for the "search view" to the "coverage note view." It has the same context menu item and the same shortcut.

##### Adding the 'general' coverage note item

- Open the "coverage note view". Use the "Add comment" context menu item or "Ctrl+Alt+A" shortcut to add a new coverage note item:

  ![Screenshot of the coverage note vaiew context menu item to add a general coverage note item](./coverage_note_view_context_menu_add_coverage_note_item.jpeg)

- This coverage note item will have no stored messages. It will be marked as a general coverage note item:

  ![Screenshot of the added general coverage note item](./general_coverage_note_item.jpeg)

- You can edit your coverage note item message and add further coverage note items.

##### Adding the 'files data' coverage note item

- Open the "coverage note view". Use the "Add files data comment" context menu item or "Ctrl+Alt+F" shortcut to add a corresponding type of the coverage note item:

  ![Screenshot of the coverage note view context menu item to add a files data coverage note item](./coverage_note_view_context_menu_add_files_data_coverage_note_item.jpeg)

- This coverage note item will have a set of opened dlt files in the "Messages" section:

  ![Screenshot of the added files data coverage note item](./files_data_coverage_note_item.jpeg)

- You can edit your coverage note item message and add further note items if needed.

----

### Deleting the coverage note item

- Open the "coverage note view". Select the message you want to delete. Use the "Delete comment" context menu item or 'Del' key to delete the selected message:

  ![Screenshot of the delete coverage note item context menu item](./delete_coverage_note_item_context_menu_item.jpeg)

- Confirm the deletion operation:

  ![Screenshot of the coverage note item deletion confirmation dialog](./delete_coverage_note_item_confirmation.jpeg)

- The item will be deleted:

  ![Screenshot of the deleted coverage note item](./coverage_note_item_deleted.jpeg)  

----

### Moving the coverage note item

You can reorder coverage note items within the coverage note.

#### Moving the coverage note item up

- Open the "coverage note view". Select one of the coverage note items. Select the "Move up" context menu item or the "Ctrl + ArrowUp" shortcut to move the target coverage note item up:

  ![Screenshot of the context menu item to move the target coverage note item up](./move_up_before.jpeg)

The change will be applied:

  ![Screenshot of the moved-up coverage note item up](./move_up_after.jpeg)

#### Moving the coverage note item down

Follow the instructions for moving up, but use the "Move down" context menu item or the "Ctrl + ArrowDown" shortcut instead.

----

## Working with the coverage note

### Creating a new coverage note

- Open the "coverage note view". Use the "New ..." context menu item or "Ctrl+N" shortcut to create a new coverage note:

  ![Screenshot of the coverage note view context menu item that allows to create a new coverage note](./coverage_note_context_menu_new.jpeg)

- You will be prompted in case your previous work is not saved:

  ![Screenshot of confirmation of saving not saved coverage note](./confirm_saving_unsaved_coverage_note.jpeg)

### Opening the coverage note

- Open the "coverage note view". Use the "Open ..." context menu item or "Ctrl+O" shortcut to create a new coverage note:

  ![Screenshot of the coverage note view context menu item that allows to open a coverage note file](./open_coverage_note.jpeg)

- The pop-up menu will allow you to select the target JSON file.

### Saving the coverage note

#### Save

- Open the "coverage note view". Use the "Save ..." context menu item or "Ctrl+S" shortcut to save a coverage note:

  ![Screenshot of the coverage note view context menu item that allows to save a coverage note file](./save_coverage_note.jpeg)

- If it is a new coverage note, a pop-up menu will appear, allowing you to select the target file path and name.

#### Save as

- Open the "coverage note view". Use the "Save as ..." context menu item or "Ctrl+S" shortcut to create a new coverage note:

  ![Screenshot of the coverage note view context menu item that allows to save a coverage note to a new file](./save_as_coverage_note.jpeg)

- The pop-up menu will allow you to select the target file path and name.

### Exporting the coverage note as HTML

- Open the "coverage note view". Use the "Save as ..." context menu item or "Ctrl+S" shortcut to create a new coverage note:

  ![Screenshot of the coverage note view context menu item that allows to export a coverage note to the HTML file](./export_to_html.jpeg)

- The pop-up menu will allow you to select the target file path and name.

- The resulting HTML file will look like this:

  ![Screenshot of the coverage note in the HTML format](./html_coverage_note.jpeg)

- Click on items in the table to switch between the coverage note item details.

----

[**Go to the previous page**](../../README.md)
