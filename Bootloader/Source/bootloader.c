#include "bootloader.h"

EFI_STATUS efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
  EFI_STATUS Status;
  EFI_GRAPHICS_OUTPUT_PROTOCOL *GraphicsOutputProtocol = NULL;
  EFI_FILE *RootFileSystem;

  EFI_PHYSICAL_ADDRESS *KernelEntryPoint = 0;
  EFI_MEMORY_DESCRIPTOR *MemoryMap = NULL;
  UINTN MemoryMapKey = 0;
  UINTN MemoryMapSize = 0;
  UINTN DescriptorSize;
  UINT32 DescriptorVersion;

  void (*KernelEntry)(KernelBootInfo * BootInfo);

  KernelBootInfo BootInfo;

  EFI_INPUT_KEY InputKey;

  SerialService.protocol = NULL;
  FileSystemService.protocol = NULL;

  InitializeLib(ImageHandle, SystemTable);

  Print(L"Welcome to the MyOS Bootloader v0.1\n");

  Status = uefi_call_wrapper(gBS->SetWatchdogTimer, 4, 0, 0, 0, NULL);

  if (check_for_fatal_error(Status, L"Error disabling the Watchdog Timer."))
  {
    return Status;
  }

  Status = uefi_call_wrapper(ST->ConIn->Reset, 2, SystemTable->ConIn, FALSE);

  if (check_for_fatal_error(Status, L"Error reseting the console input."))
  {
    return Status;
  }

  Status = InitSerialService();

  if (check_for_fatal_error(Status, L"Error intializing Serial service."))
  {
    return Status;
  }

  Status = InitGraphicsOutputService();

  if (check_for_fatal_error(Status, L"Error intializing Graphics Output service."))
  {
    return Status;
  }

  Status = uefi_call_wrapper(gBS->OpenProtocol, 6,
                             ST->ConsoleOutHandle,
                             &gEfiGraphicsOutputProtocolGuid,
                             &GraphicsOutputProtocol,
                             ImageHandle,
                             NULL,
                             EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);

  if (EFI_ERROR(Status))
  {
    debug_print_line(L"Error: Failed to open the graphics output protocol on "
                     "the active console output device: %s\n",
                     get_efi_error_message(Status));
  }

  if (GraphicsOutputProtocol)
  {
    Status = SetGraphicsMode(GraphicsOutputProtocol, TARGET_SCREEN_WIDTH, TARGET_SCREEN_HEIGHT, TARGET_PIXEL_FORMAT);

    if (EFI_ERROR(Status))
    {
      return Status;
    }
  }

  Status = InitFileSystemService();

  if (EFI_ERROR(Status))
  {
    return Status;
  }

  Status = uefi_call_wrapper(FileSystemService.protocol->OpenVolume, 2,
                             FileSystemService.protocol, &RootFileSystem);

  if (check_for_fatal_error(Status, L"Error opening root volume."))
  {
    return Status;
  }

  Status = LoadKernelImage(RootFileSystem, KERNEL_EXECUTABLE_PATH, KernelEntryPoint);

  if (EFI_ERROR(Status))
  {
    return Status;
  }

  BootInfo.video_mode_info.framebuffer_pointer =
      (VOID *)GraphicsOutputProtocol->Mode->FrameBufferBase;
  BootInfo.video_mode_info.horizontal_resolution =
      GraphicsOutputProtocol->Mode->Info->HorizontalResolution;
  BootInfo.video_mode_info.vertical_resolution =
      GraphicsOutputProtocol->Mode->Info->VerticalResolution;
  BootInfo.video_mode_info.pixels_per_scaline =
      GraphicsOutputProtocol->Mode->Info->PixelsPerScanLine;

  Status = CloseGraphicOutputService();

  if (check_for_fatal_error(Status, L"Error closing the Graphics Output Service."))
  {
    return Status;
  }

  Status = GetMemoryMap(
      (VOID **)&MemoryMap,
      &MemoryMapSize,
      &MemoryMapKey,
      &DescriptorSize,
      &DescriptorVersion);

  uefi_call_wrapper(gBS->ExitBootServices, 2, ImageHandle, MemoryMapKey);

  if (check_for_fatal_error(Status, L"Error exiting boot services."))
  {
    return Status;
  }

  BootInfo.memory_map = MemoryMap;
  BootInfo.memory_map_size = MemoryMapSize;
  BootInfo.memory_map_descriptor_size = DescriptorSize;

  KernelEntry = (void (*)(KernelBootInfo *)) * KernelEntryPoint;

  KernelEntry(&BootInfo);

  return EFI_LOAD_ERROR;
}