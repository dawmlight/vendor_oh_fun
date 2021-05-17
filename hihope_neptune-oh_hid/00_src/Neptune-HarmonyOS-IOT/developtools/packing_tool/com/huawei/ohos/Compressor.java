/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.huawei.ohos;

import java.io.BufferedOutputStream;
import java.io.BufferedInputStream;
import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.nio.charset.StandardCharsets;
import java.nio.file.attribute.FileTime;
import java.nio.file.Files;
import java.util.Locale;
import java.util.regex.Matcher;
import java.util.regex.Pattern;
import java.util.zip.CRC32;
import java.util.zip.CheckedOutputStream;
import java.util.zip.ZipInputStream;
import java.util.zip.ZipEntry;
import java.util.zip.ZipFile;
import java.util.zip.ZipOutputStream;
import java.util.Enumeration;
import java.util.ArrayList;
import java.util.List;
import java.util.Arrays;

/**
 * bundle compressor class, compress file and directory.
 *
 * @author huzeqi
 * @since 2019-10-25
 */
public class Compressor {
    private static final String JSON_SUFFIX = ".json";
    private static final String INFO_SUFFIX = ".info";
    private static final String HAP_SUFFIX = ".hap";
    private static final String PNG_SUFFIX = ".png";
    private static final String UPPERCASE_PNG_SUFFIX = ".PNG";
    private static final String CONFIG_JSON = "config.json";
    private static final String CODE = "code";
    private static final String NAME = "name";
    private static final String NULL_DIR_NAME = "";
    private static final String RES_DIR_NAME = "res/";
    private static final String RESOURCES_DIR_NAME = "resources/";
    private static final String LIBS_DIR_NAME = "libs/";
    private static final String ASSETS_DIR_NAME = "assets/";
    private static final String SO_DIR_NAME = "maple/";
    private static final String SO_ARM64_DIR_NAME = "maple/arm64/";
    private static final String LINUX_FILE_SEPARATOR = "/";
    private static final String DISTRO = "distro";
    private static final String FORMS = "forms";
    private static final String MODULE_NAME = "module-name";
    private static final String MODULE_NAME_NEW = "moduleName";
    private static final String JSON_END = "}";
    private static final String SEMICOLON = "\"";
    private static final String APPLICATION = "deviceConfig";
    private static final String COMPRESS_NATIVE_LIBS = "compressNativeLibs";
    private static final String SHARED_LIBS_DIR_NAME = "shared_libs/";
    private static final String DEVICE_TYPE = "deviceType";
    private static final String DEVICE_TYPE_FITNESSWATCH = "fitnessWatch";
    private static final String DEVICE_TYPE_FITNESSWATCH_NEW = "liteWearable";
    private static final String COLON = ":";
    private static final String COMMA = ",";
    private static final String ENTRYCARD_NAME = "EntryCard/";
    private static final String PACKINFO_NAME = "pack.info";
    private static final String ENTRYCARD_BASE_NAME = "base";
    private static final String ENTRYCARD_SNAPSHOT_NAME = "snapshot";
    private static final String VERTICAL = "vertical";
    private static final String HORIZONTAL = "horizontal";
    private static final String CAR = "car";
    private static final String TV = "tv";
    private static final String SDIP = "sdip";
    private static final String MDIP = "mdip";
    private static final String WEARABLE = "wearable";
    private static final String PIC_1X2 = "1x2";
    private static final String PIC_2X2 = "2x2";
    private static final String PIC_2X4 = "2x4";
    private static final String PIC_4X4 = "4x4";


    // set timestamp to get fixed MD5
    private static final long FILE_TIME = 1546272000000L;

    // set buffer size of each read
    private static final int BUFFER_SIZE = 10 * 1024;
    private static final long TOO_MANY_SIZE = 1024;
    private static final long TOO_BIG_SIZE = 0x6400000;
    private static final Log LOG = new Log(Compressor.class.toString());
    private static String versionCode = "";
    private static String versionName = "";

    private ZipOutputStream zipOut = null;

    private List<String> list = new ArrayList<String>();
    private List<String> formNamesList = new ArrayList<String>();
    private List<String> fileNameList = new ArrayList<String>();
    private List<String> supportDimensionsList = Arrays.asList(PIC_1X2, PIC_2X4, PIC_4X4);

    /**
     * start compress.
     * file orders as follows:
     * for hap: 1.config.json 2.lib 3.res 4.assets 5.*.so 6.*.dex 7.*.apk 8.resources.index
     * for app: 1.certificate 2.signature 3.pack.info 4.hap (1 and 2 may not be used)
     *
     * @param utility common data
     * @return compressProcess if compress succeed
     */
    public boolean compressProcess(Utility utility) {
        boolean compressResult = true;
        File destFile = new File(utility.getOutPath());

        // if out file directory not exist, mkdirs.
        File outParentFile = destFile.getParentFile();
        if ((outParentFile != null) && (!outParentFile.exists())) {
            if (!outParentFile.mkdirs()) {
                LOG.error("Compressor::compressProcess create out file parent directory failed!");
                return false;
            }
        }

        FileOutputStream fileOut = null;
        CheckedOutputStream checkedOut = null;
        try {
            fileOut = new FileOutputStream(destFile);
            checkedOut = new CheckedOutputStream(fileOut, new CRC32());
            zipOut = new ZipOutputStream(checkedOut);
            if (Utility.MODE_HAP.equals(utility.getMode())) {
                compressHapMode(utility);
            } else if (Utility.MODE_HAR.equals(utility.getMode())) {
                compressHarMode(utility);
            } else if (Utility.MODE_APP.equals(utility.getMode())) {
                compressAppMode(utility);
            } else {
                compressPackResMode(utility);
            }
        } catch (FileNotFoundException exception) {
            compressResult = false;
            LOG.error("Compressor::compressProcess file not found exception" + exception.getMessage());
        } catch (BundleException ignored) {
            compressResult = false;
            LOG.error("Compressor::compressProcess Bundle exception");
        } finally {
            closeZipOutputStream();
            Utility.closeStream(zipOut);
            Utility.closeStream(checkedOut);
            Utility.closeStream(fileOut);
        }

        // if compress failed, delete out file.
        if (!compressResult) {
            LOG.error("Compressor::compressProcess compress failed!");
            if (!destFile.delete()) {
                LOG.error("Compressor::compressProcess delete dest file failed");
            }
        }
        return compressResult;
    }

    /**
     * compress in hap mode.
     *
     * @param utility common data
     * @throws BundleException FileNotFoundException|IOException.
     */
    private void compressHapMode(Utility utility) throws BundleException {
        pathToFile(utility, utility.getJsonPath(), NULL_DIR_NAME, false);

        pathToFile(utility, utility.getProfilePath(), NULL_DIR_NAME, false);

        if (!utility.getIndexPath().isEmpty() && !utility.getModuleName().isEmpty()) {
            String assetsPath = ASSETS_DIR_NAME + utility.getModuleName() + LINUX_FILE_SEPARATOR;
            pathToFile(utility, utility.getIndexPath(), assetsPath, false);
        }

        if (!utility.getLibPath().isEmpty()) {
            pathToFile(utility, utility.getLibPath(), LIBS_DIR_NAME, utility.isCompressNativeLibs());
        }

        if (!utility.getResPath().isEmpty() && !utility.getModuleName().isEmpty()) {
            String resPath = ASSETS_DIR_NAME + utility.getModuleName() + LINUX_FILE_SEPARATOR
                    + RESOURCES_DIR_NAME;
            if (DEVICE_TYPE_FITNESSWATCH.equals(utility.getDeviceType().replace("\"", "").trim()) ||
                    DEVICE_TYPE_FITNESSWATCH_NEW.equals(utility.getDeviceType().replace("\"", "").trim())) {
                resPath = RES_DIR_NAME;
            }
            pathToFile(utility, utility.getResPath(), resPath, false);
        }

        if (!utility.getResourcesPath().isEmpty() && !utility.getModuleName().isEmpty()) {
            String resourcesPath = ASSETS_DIR_NAME + utility.getModuleName() + LINUX_FILE_SEPARATOR
                    + RESOURCES_DIR_NAME;
            pathToFile(utility, utility.getResourcesPath(), resourcesPath, false);
        }

        if (!utility.getAssetsPath().isEmpty()) {
            pathToFile(utility, utility.getAssetsPath(), ASSETS_DIR_NAME, false);
        }

        if (!utility.getBinPath().isEmpty()) {
            pathToFile(utility, utility.getBinPath(), NULL_DIR_NAME, false);
        }

        if (!utility.getPackInfoPath().isEmpty()) {
            pathToFile(utility, utility.getPackInfoPath(), NULL_DIR_NAME, false);
        }
        compressHapModeMultiple(utility);
    }

    /**
     * compress in hap mode multiple path.
     *
     * @param utility common data
     * @throws BundleException FileNotFoundException|IOException.
     */
    private void compressHapModeMultiple(Utility utility) throws BundleException {
        for (String soPathItem : utility.getFormattedSoPathList()) {
            pathToFile(utility, soPathItem, SO_ARM64_DIR_NAME, false);
        }

        if (utility.getFormattedSoPathList().size() == 0 && !utility.getSoDir().isEmpty()) {
            pathToFile(utility, utility.getSoDir(), SO_DIR_NAME, false);
        }

        for (String soPathItem : utility.getFormattedAbilitySoPathList()) {
            pathToFile(utility, soPathItem, NULL_DIR_NAME, false);
        }

        for (String dexPathItem : utility.getFormattedDexPathList()) {
            pathToFile(utility, dexPathItem, NULL_DIR_NAME, false);
        }

        for (String abcPathItem : utility.getFormattedAbcPathList()) {
            pathToFile(utility, abcPathItem, NULL_DIR_NAME, false);
        }

        for (String apkPathItem : utility.getFormattedApkPathList()) {
            pathToFile(utility, apkPathItem, NULL_DIR_NAME, false);
        }

        for (String jarPathItem : utility.getFormattedJarPathList()) {
            pathToFile(utility, jarPathItem, NULL_DIR_NAME, false);
        }

        for (String txtPathItem : utility.getFormattedTxtPathList()) {
            pathToFile(utility, txtPathItem, NULL_DIR_NAME, false);
        }

        if (!utility.getSharedLibsPath().isEmpty()) {
            pathToFile(utility, utility.getSharedLibsPath(), SHARED_LIBS_DIR_NAME, utility.isCompressNativeLibs());
        }
    }

    /**
     * compress in har mode.
     *
     * @param utility common data
     * @throws BundleException FileNotFoundException|IOException.
     */
    private void compressHarMode(Utility utility) throws BundleException {
        pathToFile(utility, utility.getJsonPath(), NULL_DIR_NAME, false);

        if (!utility.getLibPath().isEmpty()) {
            pathToFile(utility, utility.getLibPath(), LIBS_DIR_NAME, utility.isCompressNativeLibs());
        }

        if (!utility.getResPath().isEmpty()) {
            pathToFile(utility, utility.getResPath(), RESOURCES_DIR_NAME, false);
        }

        if (!utility.getResourcesPath().isEmpty()) {
            pathToFile(utility, utility.getResourcesPath(), RESOURCES_DIR_NAME, false);
        }

        if (!utility.getAssetsPath().isEmpty()) {
            pathToFile(utility, utility.getAssetsPath(), ASSETS_DIR_NAME, false);
        }

        for (String jarPathItem : utility.getFormattedJarPathList()) {
            pathToFile(utility, jarPathItem, NULL_DIR_NAME, false);
        }

        for (String txtPathItem : utility.getFormattedTxtPathList()) {
            pathToFile(utility, txtPathItem, NULL_DIR_NAME, false);
        }
    }

    /**
     * compress in app mode.
     *
     * @param utility common data
     * @throws BundleException FileNotFoundException|IOException.
     */
    private void compressAppMode(Utility utility) throws BundleException {
        pathToFile(utility, utility.getJsonPath(), NULL_DIR_NAME, false);

        if (!utility.getCertificatePath().isEmpty()) {
            pathToFile(utility, utility.getCertificatePath(), NULL_DIR_NAME, false);
        }

        if (!utility.getSignaturePath().isEmpty()) {
            pathToFile(utility, utility.getSignaturePath(), NULL_DIR_NAME, false);
        }

        String[] outPath = utility.getOutPath().replace("\\", "/").split("/");
        if (outPath.length < 2) {
            LOG.error("Compressor::compressAppMode the outPath is invalid, length: " + outPath.length);
            return;
        }
        String[] path = utility.getOutPath().split(outPath[outPath.length - 2].toString());
        List<String> fileList = new ArrayList<>();
        for (String hapPathItem : utility.getFormattedHapPathList()) {
            String fName = hapPathItem.trim();
            String[] temp = fName.replace("\\", "/").split("/");
            if (temp.length < 1) {
                LOG.error("Compressor::compressAppMode the hap file path is invalid, length: " + temp.length);
                continue;
            }
            String[] str = temp[temp.length - 1].split("\\.");
            String outPathString = path[0] + str[0];
            fileList.add(outPathString);
            unzip(hapPathItem, outPathString);
            copyFileUsingFileStreams(utility.getPackInfoPath(), outPathString);
            zipFile(outPathString);
        }

        for (String hapPath : fileList) {
            pathToFile(utility, hapPath + HAP_SUFFIX, NULL_DIR_NAME, false);
        }

        for (String hapPath : fileList) {
            deleteFile(hapPath + HAP_SUFFIX);
        }

        if (!utility.getEntryCardPath().isEmpty()) {
            String entryCardPath = ENTRYCARD_NAME + utility.getModuleName() + LINUX_FILE_SEPARATOR
                    + ENTRYCARD_BASE_NAME + ENTRYCARD_SNAPSHOT_NAME;
            for (String entryCardPathItem : utility.getformattedEntryCardPathList()) {
                pathToFile(utility, entryCardPathItem, entryCardPath, true);
            }
        }

        if (!utility.getPackResPath().isEmpty()) {
            pathToFile(utility, utility.getPackResPath(), NULL_DIR_NAME, false);
        }

        File file = new File(utility.getPackInfoPath());
        compressFile(utility, file, NULL_DIR_NAME, false);
    }

    /**
     * zipFile
     *
     * @param utility Utility
     * @param path  path
     * @param format destFileName
     */
    private void zipFile(String path) {
        FileOutputStream outputStream = null;
        ZipOutputStream out = null;
        try {
            File destFile = new File(path + HAP_SUFFIX);
            File outParentFile = destFile.getParentFile();
            if ((outParentFile != null) && (!outParentFile.exists())) {
                if (!outParentFile.mkdirs()) {
                    LOG.error("Compressor::compressProcess create out file parent directory failed!");
                }
            }
            outputStream = new FileOutputStream(destFile);
            out = new ZipOutputStream(new CheckedOutputStream(outputStream, new CRC32()));
            out.setMethod(ZipOutputStream.STORED);
            compress(new File(path), out, NULL_DIR_NAME, true);
        } catch (FileNotFoundException ignored) {
            LOG.error("zip file not found exception");
        } finally {
            Utility.closeStream(out);
            Utility.closeStream(outputStream);
            deleteFile(path);
        }
    }

    /**
     *  copyFileUsingFileStreams pack.info
     *
     * @param source inputPath
     * @param dest outputPath
     *
     */
    private static void copyFileUsingFileStreams(String source, String dest) {
        FileInputStream input = null;
        FileOutputStream output = null;
        try {
            File inputFile = new File(source);
            File outputFile = new File(dest, PACKINFO_NAME);
            File outputFileParent = outputFile.getParentFile();
            if (!outputFileParent.exists()) {
                outputFileParent.mkdirs();
            }
            if (!outputFile.exists()) {
                outputFile.createNewFile();
            }
            input = new FileInputStream(inputFile);
            output = new FileOutputStream(outputFile);
            byte[] buf = new byte[1024];
            int bytesRead;
            while ((bytesRead = input.read(buf)) != -1) {
                output.write(buf, 0, bytesRead);
            }
            input.close();
            output.close();
        } catch (FileNotFoundException ignored) {
            LOG.error("copy file not found exception" + ignored.toString());
        } catch (IOException msg) {
            LOG.error("IOException : " + msg.getMessage());
        } finally {
            Utility.closeStream(input);
            Utility.closeStream(output);
        }
    }


    /**
     * unzip hap package to path
     *
     * @param hapPath zip file
     * @param destDir path after unzip file
     */
    public static void unzip(final String hapPath, final String destDir) {
        File file = new File(destDir);
        if (!file.exists()) {
            file.mkdirs();
        }
        ZipFile zipFile = null;
        BufferedInputStream bis = null;
        BufferedOutputStream bos = null;
        FileOutputStream fos = null;
        try {
            zipFile = new ZipFile(hapPath);
            Enumeration<? extends ZipEntry> entries = zipFile.entries();
            int entriesNum = 0;
            while (entries.hasMoreElements()) {
                entriesNum++;
                if (entriesNum > TOO_MANY_SIZE) {
                    throw new IOException("dataTransferByInput failed entry num is too many");
                }
                ZipEntry entry = entries.nextElement();
                if (entry == null) {
                    continue;
                }
                if (entry.isDirectory()) {
                    new File(destDir + File.separator + entry.getName()).mkdirs();
                    continue;
                }
                bis = new BufferedInputStream(zipFile.getInputStream(entry));
                File newFile = new File(destDir + File.separator + entry.getName());
                File parent = newFile.getParentFile();
                if (parent != null && (!parent.exists())) {
                    parent.mkdirs();
                }
                fos = new FileOutputStream(newFile);
                bos = new BufferedOutputStream(fos, BUFFER_SIZE);
                writeFile(bis, bos, entry);
                bos.flush();
                bos.close();
                fos.close();
                bis.close();
            }
        } catch (FileNotFoundException ignored) {
            LOG.error("unzip file not found exception");
        } catch (IOException msg) {
            LOG.error("unzip IOException : " + msg.getMessage());
        } finally {
            Utility.closeStream(bos);
            Utility.closeStream(fos);
            Utility.closeStream(bis);
            Utility.closeStream(zipFile);
        }
    }

    /**
     *  unzipWriteFile
     *
     * @param bis BufferedInputStream
     * @param bos BufferedOutputStream
     * @param entry ZipEntry
     * @throws IOException IO
     */
    private static void writeFile(BufferedInputStream bis, BufferedOutputStream bos,
                                ZipEntry entry) throws IOException {
        int count;
        int total = 0;
        byte[] data = new byte[BUFFER_SIZE];
        while ((count = bis.read(data, 0, BUFFER_SIZE)) != -1) {
            bos.write(data, 0, count);
            total += count;
            if (total > TOO_BIG_SIZE) {
                LOG.error("FileUtils::unzip exception: " + entry.getName() + " is too big.");
                throw new IOException("dataTransferByInput failed entry num is too many");
            }
        }
    }

    /**
     * delete file
     *
     * @param path file path which will be deleted
     */
    private static void deleteFile(final String path) {
        File file = new File(path);
        if (file.exists()) {
            if (file.isDirectory()) {
                File[] files = file.listFiles();
                for (int i = 0; i < files.length; i++) {
                    deleteFile(files[i].toString());
                }
            }
            file.delete();
        }
    }

    /**
     * compress in res mode.
     *
     * @param utility common data
     * @throws BundleException FileNotFoundException|IOException.
     */
    private void compressPackResMode(Utility utility) throws BundleException {
        if (!utility.getPackInfoPath().isEmpty()) {
            File file = new File(utility.getPackInfoPath());
            infoSpecialProcess(utility, file);
        }
        if (!utility.getEntryCardPath().isEmpty()) {
            getFileList(utility.getEntryCardPath());
            boolean isSatisfies = true;
            for (String fileName : fileNameList) {
                if (fileName.endsWith(PNG_SUFFIX) || fileName.endsWith(UPPERCASE_PNG_SUFFIX)) {
                    String fName = fileName.trim();
                    String[] temp = fName.replace("\\", "/").split("/");
                    if (temp.length < 4) {
                        LOG.error("Compressor::compressPackResMode the hap file path is invalid, length: "
                            + temp.length);
                        continue;
                    }
                    String fileModelName = temp[temp.length - 4];
                    if (!isModelName(fileModelName)) {
                        LOG.error("Compressor::compressProcess compress failed ModelName Error!");
                        isSatisfies = false;
                        break;
                    }
                    String fileLanguageCountryName = temp[temp.length - 3];
                    if (!isLanguageCountry(fileLanguageCountryName)) {
                        LOG.error("Compressor::compressProcess compress failed LanguageCountryName Error!");
                        isSatisfies = false;
                        break;
                    }
                    String filePicturingName = temp[temp.length - 1];
                    if (!filePicturingName.contains(PIC_2X2) && !isPicturing(filePicturingName)) {
                        LOG.error("Compressor::compressProcess compress failed PicturingName Error!");
                        isSatisfies = false;
                        break;
                    }

                } else {
                    isSatisfies = false;
                    LOG.error("Compressor::compressProcess compress failed No image in PNG format is found!");
                }
            }
            if (isSatisfies) {
                pathToFile(utility, utility.getEntryCardPath(), ENTRYCARD_NAME, false);
            }
        }
    }

    /**
     * Check whether modelname meets specifications.
     *
     * @param name modelName
     * @return false and true
     */
    private boolean isModelName(String name) {
        for (String listName : list) {
            if (name.equals(listName)) {
                return true;
            }
        }
        return false;
    }

    /**
     * Check whether languageCountryName meets specifications.
     *
     * @param name languageCountryName
     * @return false and true
     */
    private boolean isLanguageCountry(String name) {
        if (!name.equals(ENTRYCARD_BASE_NAME)) {
            boolean isLanguage = false;
            String[] str = name.split("-");
            if (str.length > 1) {
                Locale[] ls = Locale.getAvailableLocales();
                for (int i = 0; i < ls.length; i++) {
                    if (ls[i].toString().equals(str[0])) {
                        isLanguage = true;
                    }
                }
                if (str[1].equals( VERTICAL ) || str[1].equals( HORIZONTAL )) {
                    isLanguage = true;
                }
                if (str[2].equals( CAR ) || str[2].equals( TV ) || str[2].equals( WEARABLE )) {
                    isLanguage = true;
                }
                if (str[3].equals( SDIP ) || str[3].equals( MDIP )) {
                    isLanguage = true;
                }
                return isLanguage;
            } else {
                return false;
            }
        } else {
            return true;
        }
    }

    /**
     * Check whether picturingName meets specifications.
     *
     * @param name picturingName
     * @return false and true
     */
    private boolean isPicturing(String name) {
        boolean isSpecifications = false;
        if (name == null || name.isEmpty()) {
            return isSpecifications;
        }
        if (!name.endsWith(PNG_SUFFIX) && !name.endsWith(UPPERCASE_PNG_SUFFIX)) {
            LOG.error("isPicturing: the suffix is not .png or .PNG");
            return false;
        }
        int delimiterIndex = name.lastIndexOf("-");
        if (delimiterIndex < 0) {
            LOG.error("isPicturing: the entry card naming format is invalid and should be separated by '-'!");
            return false;
        }
        String formName = name.substring(0, delimiterIndex);
        if (!formNamesList.contains(formName)) {
            LOG.error("isPicturing: the name is not same as formName, name: " + formName);
            return false;
        }
        String dimension = name.substring(delimiterIndex + 1, name.lastIndexOf("."));
        return supportDimensionsList.contains(dimension);
    }

    /**
     * get file list in filePath
     *
     * @param filePath file path
     * @param fileList file path in arrayList
     */
    private void getFileList(final String filePath) {
        File file = new File(filePath);
        if (!file.exists()) {
            LOG.error("getFileList: file is not exists");
            return;
        }
        File[] files = file.listFiles();
        if (files == null) {
            LOG.error("getFileList: no file in this file path");
            return;
        }
        for (File f : files) {
            try {
                if (f.isFile()) {
                    if (f.getName().endsWith(".DS_Store")) {
                        deleteFile(f.getCanonicalPath());
                        continue;
                    }
                    fileNameList.add(f.getCanonicalPath());
                } else if (f.isDirectory()) {
                    getFileList(f.getCanonicalPath());
                } else {
                    LOG.error("It's not file or directory!");
                }
            } catch (IOException msg) {
                LOG.error("IOException error: " + msg.getMessage());
                return;
            }
        }
    }

    /**
     * compress file or directory.
     *
     * @param utility       common data
     * @param path          create new file by path
     * @param baseDir       base path for file
     * @param isCompression if need compression
     * @throws BundleException FileNotFoundException|IOException.
     */
    private void pathToFile(Utility utility, String path, String baseDir, boolean isCompression)
            throws BundleException {
        if (path.isEmpty()) {
            return;
        }
        File fileItem = new File(path);
        if (fileItem.isDirectory()) {
            File[] files = fileItem.listFiles();
            if (files == null) {
                return;
            }
            for (File file : files) {
                if (file.isDirectory()) {
                    compressDirectory(utility, file, baseDir, isCompression);
                } else if (isCompression) {
                    compressFile(utility, file, baseDir, isCompression);
                } else {
                    compressFile(utility, file, baseDir, isCompression);
                }
            }
        } else {
            compressFile(utility, fileItem, baseDir, isCompression);
        }
    }

    /**
     * compress file directory.
     *
     * @param utility       common data
     * @param dir           file directory
     * @param baseDir       current directory name
     * @param isCompression if need compression
     * @throws BundleException FileNotFoundException|IOException.
     */
    private void compressDirectory(Utility utility, File dir, String baseDir, boolean isCompression)
            throws BundleException {
        File[] files = dir.listFiles();
        if (files == null) {
            return;
        }
        for (File file : files) {
            if (file.isDirectory()) {
                compressDirectory(utility, file, baseDir + dir.getName() + File.separator, isCompression);
            } else {
                compressFile(utility, file, baseDir + dir.getName() + File.separator, isCompression);
            }
        }
    }

    /**
     * compress pack.info
     *
     * @param sourceFile source
     * @param zos ZipOutputStream
     * @param name filename
     * @param KeepDirStructure Empty File
     */
    private void compress(File sourceFile, ZipOutputStream zos, String name,
                                boolean KeepDirStructure) {
        FileInputStream in = null;
        try {
            byte[] buf = new byte[BUFFER_SIZE];
            if (sourceFile.isFile()) {
                ZipEntry zipEntry = getStoredZipEntry(sourceFile, name);
                zos.putNextEntry(zipEntry);
                in = new FileInputStream(sourceFile);
                int len;
                while ((len = in.read(buf)) != -1) {
                    zos.write(buf, 0, len);
                }
                zos.closeEntry();
            } else {
                File[] listFiles = sourceFile.listFiles();
                if (listFiles == null || listFiles.length == 0) {
                    if (KeepDirStructure) {
                        if (!name.isEmpty()) {
                            ZipEntry zipEntry = getStoredZipEntry(sourceFile, name + "/");
                            zos.putNextEntry(zipEntry);
                        } else {
                            ZipEntry zipEntry = getStoredZipEntry(sourceFile, name);
                            zos.putNextEntry(zipEntry);
                        }
                        zos.closeEntry();
                    }
                } else {
                    for (File file : listFiles) {
                        if (KeepDirStructure) {
                            isNameEmpty(zos, name, KeepDirStructure, file);
                        } else {
                            compress(file, zos, file.getName(), KeepDirStructure);
                        }
                    }
                }
            }
        } catch (FileNotFoundException ignored) {
            LOG.error("Compressor::compressFile file not found exception");
        } catch (IOException exception) {
            LOG.error("Compressor::compressFile io exception: " + exception.getMessage());
        } catch (BundleException bundleException) {
            LOG.error("Compressor::compressFile bundle exception" + bundleException.getMessage());
        } finally {
            Utility.closeStream(in);
        }
    }

    private ZipEntry getStoredZipEntry(File sourceFile, String name) throws BundleException {
        ZipEntry zipEntry = new ZipEntry(name);
        zipEntry.setMethod(ZipEntry.STORED);
        zipEntry.setCompressedSize(sourceFile.length());
        zipEntry.setSize(sourceFile.length());
        CRC32 crc = getCrcFromFile(sourceFile);
        zipEntry.setCrc(crc.getValue());
        FileTime fileTime = FileTime.fromMillis(FILE_TIME);
        zipEntry.setLastAccessTime(fileTime);
        zipEntry.setLastModifiedTime(fileTime);
        return zipEntry;
    }

    private CRC32 getCrcFromFile(File file) throws BundleException {
        FileInputStream fileInputStream = null;
        CRC32 crc = new CRC32();
        try {
            fileInputStream = new FileInputStream(file);
            byte[] buffer = new byte[BUFFER_SIZE];

            int count = fileInputStream.read(buffer);
            while (count > 0) {
                crc.update(buffer, 0, count);
                count = fileInputStream.read(buffer);
            }
        } catch (FileNotFoundException ignored) {
            LOG.error("Uncompressor::getCrcFromFile file not found exception");
            throw new BundleException("Get Crc from file failed");
        } catch (IOException exception) {
            LOG.error("Uncompressor::getCrcFromFile io exception: " + exception.getMessage());
            throw new BundleException("Get Crc from file failed");
        } finally {
            Utility.closeStream(fileInputStream);
        }
        return crc;
    }

    /**
     * isNameEmpty
     *
     * @param zos ZipOutputStream
     * @param name filename
     * @param KeepDirStructure KeepDirStructure
     * @param file file
     */
    private void isNameEmpty(ZipOutputStream zos, String name, boolean KeepDirStructure, File file) {
        if (!name.isEmpty()) {
            compress(file, zos, name + "/" + file.getName(), KeepDirStructure);
        } else {
            compress(file, zos, file.getName(), KeepDirStructure);
        }
    }


    /**
     * compress process.
     *
     * @param utility       common data
     * @param srcFile       source file to zip
     * @param baseDir       current directory name of file
     * @param isCompression if need compression
     * @throws BundleException FileNotFoundException|IOException.
     */
    private void compressFile(Utility utility, File srcFile, String baseDir, boolean isCompression)
            throws BundleException {
        BufferedInputStream bufferedInputStream = null;
        FileInputStream fileInputStream = null;
        try {
            String entryName = (baseDir + srcFile.getName()).replace(File.separator, LINUX_FILE_SEPARATOR);
            ZipEntry zipEntry = new ZipEntry(entryName);
            if (!checkVersionInHaps(utility, srcFile, entryName)) {
                LOG.error("Compressor::compressFile file checkVersionCodeInHaps failed");
                throw new BundleException("Compressor::compressFile There are some haps with different version code!");
            }
            if (srcFile.getName().toLowerCase(Locale.ENGLISH).endsWith(JSON_SUFFIX)) {
                zipEntry.setMethod(ZipEntry.STORED);
                jsonSpecialProcess(utility, srcFile, zipEntry);
                return;
            }

            if (isCompression) {
                zipEntry.setMethod(ZipEntry.DEFLATED);
            } else {
                zipEntry.setMethod(ZipEntry.STORED);

                // update size
                zipEntry.setCompressedSize(srcFile.length());
                zipEntry.setSize(srcFile.length());

                // update crc
                CRC32 crc = getCrcFromFile(utility, srcFile);
                zipEntry.setCrc(crc.getValue());
            }

            // update fileTime
            FileTime fileTime = FileTime.fromMillis(FILE_TIME);
            zipEntry.setLastAccessTime(fileTime);
            zipEntry.setLastModifiedTime(fileTime);

            zipOut.putNextEntry(zipEntry);
            byte[] data = new byte[BUFFER_SIZE];
            fileInputStream = new FileInputStream(srcFile);
            bufferedInputStream = new BufferedInputStream(fileInputStream);

            int count = bufferedInputStream.read(data);
            while (count > 0) {
                zipOut.write(data, 0, count);
                count = bufferedInputStream.read(data);
            }
        } catch (FileNotFoundException ignored) {
            LOG.error("Compressor::compressFile file not found exception");
            throw new BundleException("CoompressFile failed");
        } catch (IOException exception) {
            LOG.error("Compressor::compressFile io exception: " + exception.getMessage());
            throw new BundleException("CoompressFile failed");
        } finally {
            Utility.closeStream(bufferedInputStream);
            Utility.closeStream(fileInputStream);
        }
    }

    /**
     * check version code in haps.
     *
     * @param utility common data
     * @param srcFile source file to zip
     * @param baseDir current directory name of file
     * @return true is for successful and false is for failed
     * @throws BundleException FileNotFoundException|IOException.
     */
    private boolean checkVersionInHaps(Utility utility, File srcFile, String baseDir) throws BundleException {
        try {
            String fileStr = srcFile.getName();
            if ((fileStr == null) || (fileStr.isEmpty())) {
                throw new BundleException("Compressor::checkVersionInHaps get file name failed");
            }
            if (!fileStr.toLowerCase(Locale.ENGLISH).endsWith(HAP_SUFFIX)) {
                return true;
            }
            for (String hapPath : utility.getFormattedHapPathList()) {
                if ((hapPath == null) || (hapPath.isEmpty()) || (!hapPath.contains(baseDir))) {
                    continue;
                }
                String configJson = obtainVersion(srcFile, hapPath);
                String str = obtainInnerVersionCode(configJson);
                if ((str == null) || (str.isEmpty())) {
                    LOG.error("Compressor::checkVersionInHaps version code is null or empty");
                    return false;
                }
                if (!versionCode.isEmpty() && !versionCode.equals(str)) {
                    LOG.error("Compressor::checkVersionInHaps some haps with different version code");
                    return false;
                }
                String nameStr = obtainInnerVersionName(configJson);
                if ((nameStr == null) || (nameStr.isEmpty())) {
                    LOG.error("Compressor::checkVersionInHaps version name is null or empty");
                    return false;
                }
                if (!versionName.isEmpty() && !versionName.equals(nameStr)) {
                    LOG.error("Compressor::checkVersionInHaps some haps with different version name");
                    return false;
                }
                versionCode = str;
                versionName = nameStr;
            }
        } catch (BundleException exception) {
            LOG.error("Compressor::checkVersionInHaps io exception: " + exception.getMessage());
            throw new BundleException("Compressor::checkVersionInHaps failed");
        }
        return true;
    }

    /**
     * get CRC32 from file.
     *
     * @param utility common data
     * @param file    source file
     * @return CRC32
     * @throws BundleException FileNotFoundException|IOException.
     */
    private CRC32 getCrcFromFile(Utility utility, File file) throws BundleException {
        FileInputStream fileInputStream = null;
        CRC32 crc = new CRC32();
        try {
            fileInputStream = new FileInputStream(file);
            byte[] buffer = new byte[BUFFER_SIZE];

            int count = fileInputStream.read(buffer);
            while (count > 0) {
                crc.update(buffer, 0, count);
                count = fileInputStream.read(buffer);
            }
        } catch (FileNotFoundException ignored) {
            LOG.error("Compressor::getCrcFromFile file not found exception");
            throw new BundleException("Get Crc from file failed");
        } catch (IOException exception) {
            LOG.error("Compressor::getCrcFromFile io exception: " + exception.getMessage());
            throw new BundleException("Get Crc from file failed");
        } finally {
            Utility.closeStream(fileInputStream);
        }
        return crc;
    }

    private void infoSpecialProcess(Utility utility, File srcFile)
            throws BundleException {
        FileInputStream fileInputStream = null;
        BufferedReader bufferedReader = null;
        InputStreamReader inputStreamReader = null;

        try {
            fileInputStream = new FileInputStream(srcFile);
            inputStreamReader = new InputStreamReader(fileInputStream, StandardCharsets.UTF_8);
            bufferedReader = new BufferedReader(inputStreamReader);
            bufferedReader.mark((int) srcFile.length() + 1);
            // parse moduleName from config.json
            parsePackModuleName(bufferedReader, utility);
            bufferedReader.reset();
            parsePackFormName(bufferedReader, utility);
            bufferedReader.reset();
            parseDeviceType(bufferedReader, utility);
            bufferedReader.reset();

            Pattern pattern = Pattern.compile(System.lineSeparator());
            String str = bufferedReader.readLine();
            StringBuilder builder = new StringBuilder();
            while (str != null) {
                Matcher matcher = pattern.matcher(str.trim());
                String dest = matcher.replaceAll("");
                builder.append(dest);
                str = bufferedReader.readLine();
            }
        } catch (IOException exception) {
            LOG.error("Compressor::jsonSpecialProcess io exception: " + exception.getMessage());
            throw new BundleException("Json special process failed");
        } finally {
            Utility.closeStream(bufferedReader);
            Utility.closeStream(inputStreamReader);
            Utility.closeStream(fileInputStream);
        }
    }

    /**
     * trim and remove "\r\n" in *.json file.
     *
     * @param utility common data
     * @param srcFile file input
     * @param entry   zip file entry
     * @throws BundleException FileNotFoundException|IOException.
     */
    private void jsonSpecialProcess(Utility utility, File srcFile, ZipEntry entry)
            throws BundleException {
        FileInputStream fileInputStream = null;
        BufferedReader bufferedReader = null;
        InputStreamReader inputStreamReader = null;

        try {
            fileInputStream = new FileInputStream(srcFile);
            inputStreamReader = new InputStreamReader(fileInputStream, StandardCharsets.UTF_8);
            bufferedReader = new BufferedReader(inputStreamReader);
            bufferedReader.mark((int) srcFile.length() + 1);
            // parse moduleName from config.json
            parseModuleName(bufferedReader, utility);
            bufferedReader.reset();
            parseCompressNativeLibs(bufferedReader, utility);
            bufferedReader.reset();
            parseDeviceType(bufferedReader, utility);
            bufferedReader.reset();

            Pattern pattern = Pattern.compile(System.lineSeparator());
            String str = bufferedReader.readLine();
            StringBuilder builder = new StringBuilder();
            while (str != null) {
                Matcher matcher = pattern.matcher(str.trim());
                String dest = matcher.replaceAll("");
                builder.append(dest);
                str = bufferedReader.readLine();
            }
            byte[] trimJson = builder.toString().getBytes(StandardCharsets.UTF_8);

            // update crc
            CRC32 crc = new CRC32();
            crc.update(trimJson);
            entry.setCrc(crc.getValue());

            // update size
            entry.setSize(trimJson.length);
            entry.setCompressedSize(trimJson.length);

            // update fileTime
            FileTime fileTime = FileTime.fromMillis(FILE_TIME);
            entry.setLastAccessTime(fileTime);
            entry.setLastModifiedTime(fileTime);

            // compress data
            zipOut.putNextEntry(entry);
            zipOut.write(trimJson);
        } catch (IOException exception) {
            LOG.error("Compressor::jsonSpecialProcess io exception: " + exception.getMessage());
            throw new BundleException("Json special process failed");
        } finally {
            Utility.closeStream(bufferedReader);
            Utility.closeStream(inputStreamReader);
            Utility.closeStream(fileInputStream);
        }
    }

    /**
     * get string of config.json from hap file.
     *
     * @param file    source file
     * @param baseDir current directory name of file
     * @return string of config.json
     * @throws BundleException FileNotFoundException|IOException.
     */
    private String obtainVersion(File srcFile, String baseDir) throws BundleException {
        ZipFile zipFile = null;
        FileInputStream zipInput = null;
        InputStream in = null;
        ZipInputStream zin = null;
        InputStream inputStream = null;
        InputStreamReader reader = null;
        BufferedReader br = null;
        ZipEntry entry = null;
        String versionStr = "";
        try {
            zipFile = new ZipFile(srcFile);
            zipInput = new FileInputStream(baseDir);
            in = new BufferedInputStream(zipInput);
            zin = new ZipInputStream(in);
            while ((entry = zin.getNextEntry()) != null) {
                if (entry.getName().toLowerCase().equals(CONFIG_JSON)) {
                    inputStream = zipFile.getInputStream(entry);
                    reader = new InputStreamReader(inputStream);
                    br = new BufferedReader(reader);
                    if (br != null) {
                        versionStr = br.readLine();
                        break;
                    }
                }
            }
        } catch (IOException exception) {
            LOG.error("Compressor::ObtainVersionCode io exception: " + exception.getMessage());
            throw new BundleException("Compressor::ObtainVersionCode failed");
        } finally {
            Utility.closeStream(zipFile);
            Utility.closeStream(zipInput);
            Utility.closeStream(in);
            Utility.closeStream(zin);
            Utility.closeStream(inputStream);
            Utility.closeStream(reader);
            Utility.closeStream(br);
        }
        return versionStr;
    }

    /**
     * get version code from hap file.
     *
     * @param configJson config.json from hap file
     * @return version code
     * @throws BundleException FileNotFoundException|IOException.
     */
    private String obtainInnerVersionCode(String configJson) throws BundleException {
        try {
            if (configJson != null) {
                int indexOfCode = configJson.indexOf(CODE);
                if (indexOfCode <= 0) {
                    LOG.error("Compressor::ObtainInnerVersionCode obtain index failed");
                    throw new BundleException("Compressor::ObtainInnerVersionCode obtain index failed");
                }
                int index = configJson.indexOf(COLON, indexOfCode);
                if (index <= 0) {
                    LOG.error("Compressor::ObtainInnerVersionCode obtain index failed");
                    throw new BundleException("Compressor::ObtainInnerVersionCode obtain index failed");
                }
                int lastIndex = configJson.indexOf(COMMA, index);
                if (lastIndex <= 0) {
                    LOG.error("Compressor::ObtainInnerVersionCode obtain index failed");
                    throw new BundleException("Compressor::ObtainInnerVersionCode obtain index failed");
                }
                String code = configJson.substring(index + 1, lastIndex);
                if (code == null || code.isEmpty()) {
                    LOG.error("Compressor::ObtainInnerVersionCode code is null or empty");
                    throw new BundleException("Compressor::ObtainInnerVersionCode failed due to null or empty code!");
                }
                return code.trim();
            }
        } catch (BundleException exception) {
            LOG.error("Compressor::ObtainInnerVersionCode io exception: " + exception.getMessage());
            throw new BundleException("Compressor::ObtainInnerVersionCode failed");
        }
        return "";
    }

    /**
     * get version name from hap file.
     *
     * @param configJson config.json from hap file
     * @return version name
     * @throws BundleException FileNotFoundException|IOException.
     */
    private String obtainInnerVersionName(String configJson) throws BundleException {
        try {
            if (configJson != null) {
                int indexOfCode = configJson.indexOf(NAME);
                if (indexOfCode <= 0) {
                    LOG.error("Compressor::obtainInnerVersionName obtain index failed");
                    throw new BundleException("Compressor::obtainInnerVersionName obtain index failed");
                }
                int index = configJson.indexOf(COLON, indexOfCode);
                if (index <= 0) {
                    LOG.error("Compressor::obtainInnerVersionName obtain index failed");
                    throw new BundleException("Compressor::obtainInnerVersionName obtain index failed");
                }
                int lastIndex = configJson.indexOf(JSON_END, index);
                if (lastIndex <= 0) {
                    LOG.error("Compressor::obtainInnerVersionName obtain index failed");
                    throw new BundleException("Compressor::obtainInnerVersionName obtain index failed");
                }
                String name = configJson.substring(index + 1, lastIndex);
                if (name == null || name.isEmpty()) {
                    LOG.error("Compressor::obtainInnerVersionName name is null or empty");
                    throw new BundleException("Compressor::obtainInnerVersionName failed due to null or empty name!");
                }
                return name.trim();
            }
        } catch (BundleException exception) {
            LOG.error("Compressor::obtainInnerVersionName io exception: " + exception.getMessage());
            throw new BundleException("Compressor::obtainInnerVersionName failed");
        }
        return "";
    }

    /**
     * Parse module name from config.json
     *
     * @param bufferedReader config.json buffered Reader
     * @param utility        common data
     * @throws BundleException IOException
     */
    private void parseModuleName(BufferedReader bufferedReader, Utility utility) throws BundleException {
        String lineStr = null;
        boolean isDistroStart = false;
        try {
            while ((lineStr = bufferedReader.readLine()) != null) {
                if (!isDistroStart) {
                    if (lineStr.contains(DISTRO)) {
                        isDistroStart = true;
                    }
                    continue;
                }
                if (lineStr.contains(JSON_END)) {
                    continue;
                }
                if (lineStr.contains(MODULE_NAME_NEW) || lineStr.contains(MODULE_NAME)) {
                    getModuleNameFromString(lineStr, utility);
                    break;
                }
            }
        } catch (IOException exception) {
            LOG.error("Compressor::parseModuleName io exception: " + exception.getMessage());
            throw new BundleException("Parse module name failed");
        }
    }

    /**
     * Parse module name from pack.info
     *
     * @param bufferedReader pack.info buffered Reader
     * @param utility        common data
     * @throws BundleException IOException
     */
    private void parsePackModuleName(BufferedReader bufferedReader, Utility utility) throws BundleException {
        String lineStr = null;
        boolean isDistroStart = false;
        try {
            while ((lineStr = bufferedReader.readLine()) != null) {
                if (lineStr.contains(DISTRO)) {
                    continue;
                }
                if (lineStr.contains(JSON_END)) {
                    continue;
                }
                if (lineStr.contains(MODULE_NAME_NEW) || lineStr.contains(MODULE_NAME)) {
                    getModuleNameFromString(lineStr, utility);
                }
            }
        } catch (IOException exception) {
            LOG.error("Compressor::parseModuleName io exception: " + exception.getMessage());
            throw new BundleException("Parse module name failed");
        }
    }

    /**
     * Parse Forms name from pack.info
     *
     * @param bufferedReader pack.info buffered Reader
     * @param utility        common data
     * @throws BundleException IOException
     */
    private void parsePackFormName(BufferedReader bufferedReader, Utility utility) throws BundleException {
        String lineStr = null;
        boolean isDistroStart = false;
        try {
            while ((lineStr = bufferedReader.readLine()) != null) {
                if (lineStr.contains("abilities")) {
                    continue;
                }
                if (lineStr.contains(FORMS)) {
                    continue;
                }
                if (lineStr.contains(JSON_END)) {
                    continue;
                }
                if (lineStr.contains(NAME)) {
                    getNameFromString(lineStr, utility);
                }
            }
        } catch (IOException exception) {
            LOG.error("Compressor::parseModuleName io exception: " + exception.getMessage());
            throw new BundleException("Parse module name failed");
        }
    }


    /**
     * Get name from line string
     *
     * @param lineStr line string
     * @param utility common data
     * @throws BundleException StringIndexOutOfBoundsException
     */
    private void getNameFromString(String lineStr, Utility utility) throws BundleException {
        try {
            int endIndex = lineStr.lastIndexOf(SEMICOLON);
            if (endIndex <= 0) {
                LOG.error("Compressor::getModuleNameFromString field the json is not standard.");
                throw new BundleException("Parse module name failed, module-name is invalid");
            }
            int startIndex = lineStr.lastIndexOf(SEMICOLON, endIndex - 1) + 1;
            String fromsName = lineStr.substring(startIndex, endIndex);
            if (fromsName == null || fromsName.isEmpty()) {
                LOG.error("Compressor::getModuleNameFromString field module-name is empty");
                throw new BundleException("Parse module name failed, module-name is empty");
            }
            String[] nameList = fromsName.split("\\.");
            if (nameList.length <= 1) {
                formNamesList.add(fromsName);
            }
        } catch (StringIndexOutOfBoundsException exception) {
            LOG.error("Compressor::parseModuleName field module-name is fault: " + exception.getMessage());
            throw new BundleException("Parse module name failed, module-name is invalid");
        }
    }

    /**
     * Get module name from line string
     *
     * @param lineStr line string
     * @param utility common data
     * @throws BundleException StringIndexOutOfBoundsException
     */
    private void getModuleNameFromString(String lineStr, Utility utility) throws BundleException {
        try {
            int endIndex = lineStr.lastIndexOf(SEMICOLON);
            if (endIndex <= 0) {
                LOG.error("Compressor::getModuleNameFromString field the json is not standard.");
                throw new BundleException("Parse module name failed, module-name is invalid");
            }
            int startIndex = lineStr.lastIndexOf(SEMICOLON, endIndex - 1) + 1;
            String moduleName = lineStr.substring(startIndex, endIndex);
            list.add(moduleName);
            if (moduleName == null || moduleName.isEmpty()) {
                LOG.error("Compressor::getModuleNameFromString field module-name is empty");
                throw new BundleException("Parse module name failed, module-name is empty");
            }
            utility.setModuleName(moduleName);
        } catch (StringIndexOutOfBoundsException exception) {
            LOG.error("Compressor::parseModuleName field module-name is fault: " + exception.getMessage());
            throw new BundleException("Parse module name failed, module-name is invalid");
        }
    }

    private void parseCompressNativeLibs(BufferedReader bufferedReader, Utility utility) throws BundleException {
        String lineStr = null;
        try {
            while ((lineStr = bufferedReader.readLine()) != null) {
                if (lineStr.contains(COMPRESS_NATIVE_LIBS)) {
                    if (lineStr.contains(Utility.FALSE_STRING)) {
                        utility.setIsCompressNativeLibs(false);
                        break;
                    }
                }
            }
        } catch (IOException exception) {
            LOG.error("Compressor::parseCompressNativeLibs io exception: " + exception.getMessage());
            throw new BundleException("Parse compress native libs failed");
        }
    }

    /**
     * ZipOutputStream flush, closeEntry and finish.
     */
    private void closeZipOutputStream() {
        try {
            if (zipOut != null) {
                zipOut.flush();
            }
        } catch (IOException exception) {
            LOG.error("Compressor::closeZipOutputStream flush exception " + exception.getMessage());
        }
        try {
            if (zipOut != null) {
                zipOut.closeEntry();
            }
        } catch (IOException exception) {
            LOG.error("Compressor::closeZipOutputStream close entry io exception " + exception.getMessage());
        }
        try {
            if (zipOut != null) {
                zipOut.finish();
            }
        } catch (IOException exception) {
            LOG.error("Compressor::closeZipOutputStream finish exception " + exception.getMessage());
        }
    }

    /**
     * Parse device type from config.json
     *
     * @param bufferedReader config.json buffered Reader
     * @param utility        common data
     * @throws BundleException IOException
     */
    private void parseDeviceType(BufferedReader bufferedReader, Utility utility) throws BundleException {
        String lineStr = null;
        boolean isDeviceTypeStart = false;
        try {
            while ((lineStr = bufferedReader.readLine()) != null) {
                if (!isDeviceTypeStart) {
                    if (lineStr.contains(DEVICE_TYPE)) {
                        isDeviceTypeStart = true;
                    }
                    continue;
                }
                if (lineStr.contains(JSON_END)) {
                    break;
                }
                utility.setDeviceType(lineStr);
                break;
            }
        } catch (IOException exception) {
            LOG.error("Compressor::parseDeviceType io exception: " + exception.getMessage());
            throw new BundleException("Parse device type failed");
        }
    }
}
